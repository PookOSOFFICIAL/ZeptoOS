#include "ext2.h"
#include "../drv/pata.h"

static struct ext2_super_block sb;
static struct ext2_group_desc bg;
static uint32_t block_size = 1024;

static void ext2_read_block(uint32_t block_num, uint8_t* buf) {
    uint32_t sectors_per_block = block_size / 512;
    pata_read_sectors(block_num * sectors_per_block, sectors_per_block, buf);
}

static void ext2_write_block(uint32_t block_num, const uint8_t* buf) {
    uint32_t sectors_per_block = block_size / 512;
    pata_write_sectors(block_num * sectors_per_block, sectors_per_block, (uint8_t*)buf);
}

static void ext2_read_inode(uint32_t inode_num, struct ext2_inode* inode) {
    uint32_t inodes_per_group = sb.s_inodes_per_group;
    uint32_t group = (inode_num - 1) / inodes_per_group;
    uint32_t index = (inode_num - 1) % inodes_per_group;
    
    uint8_t block_buf[4096];
    uint32_t table_block = bg.bg_inode_table;
    uint32_t inode_size = sizeof(struct ext2_inode);
    uint32_t block_offset = (index * inode_size) / block_size;
    
    ext2_read_block(table_block + block_offset, block_buf);
    struct ext2_inode* inodes = (struct ext2_inode*)block_buf;
    *inode = inodes[index % (block_size / inode_size)];
}

static uint32_t ext2_read_data(struct ext2_inode* inode, uint32_t offset, uint32_t size, uint8_t* buffer) {
    uint32_t bytes_read = 0;
    uint8_t b_buf[4096];
    
    while (bytes_read < size && offset < inode->i_size) {
        uint32_t b_idx = offset / block_size;
        uint32_t b_off = offset % block_size;
        uint32_t chunk = block_size - b_off;
        if (chunk > size - bytes_read) chunk = size - bytes_read;
        if (chunk > inode->i_size - offset) chunk = inode->i_size - offset;
        
        uint32_t phys_block = inode->i_block[b_idx];
        ext2_read_block(phys_block, b_buf);
        
        for (uint32_t i = 0; i < chunk; i++) {
            buffer[bytes_read++] = b_buf[b_off + i];
        }
        offset += chunk;
    }
    return bytes_read;
}

struct ext2_dir_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char name[256];
};

static struct dirent cached_dirent;

static uint32_t ext2_node_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    struct ext2_inode inode;
    ext2_read_inode(node->inode, &inode);
    return ext2_read_data(&inode, offset, size, buffer);
}

static uint32_t ext2_node_write(struct vfs_node* node, uint32_t offset, uint32_t size, const uint8_t* buffer) {
    struct ext2_inode inode;
    ext2_read_inode(node->inode, &inode);
    uint32_t b_idx = offset / block_size;
    uint8_t b_buf[4096];
    uint32_t phys_block = inode.i_block[b_idx];
    ext2_read_block(phys_block, b_buf);
    uint32_t b_off = offset % block_size;
    for (uint32_t i = 0; i < size; i++) {
        b_buf[b_off + i] = buffer[i];
    }
    ext2_write_block(phys_block, b_buf);
    return size;
}

static struct dirent* ext2_node_readdir(struct vfs_node* node, uint32_t index) {
    struct ext2_inode inode;
    ext2_read_inode(node->inode, &inode);
    
    uint8_t b_buf[4096];
    uint32_t offset = 0;
    uint32_t current_index = 0;
    
    while (offset < inode.i_size) {
        ext2_read_block(inode.i_block[0], b_buf);
        struct ext2_dir_entry* entry = (struct ext2_dir_entry*)(b_buf + offset);
        if (entry->rec_len == 0) break;
        
        if (entry->inode != 0) {
            if (current_index == index) {
                cached_dirent.ino = entry->inode;
                cached_dirent.type = entry->file_type;
                int i = 0;
                for (; i < entry->name_len && i < 127; i++) {
                    cached_dirent.name[i] = entry->name[i];
                }
                cached_dirent.name[i] = '\0';
                return &cached_dirent;
            }
            current_index++;
        }
        offset += entry->rec_len;
    }
    return (struct dirent*)0;
}

static struct vfs_node* ext2_node_finddir(struct vfs_node* node, const char* name) {
    struct ext2_inode inode;
    ext2_read_inode(node->inode, &inode);
    
    uint8_t b_buf[4096];
    uint32_t offset = 0;
    
    while (offset < inode.i_size) {
        ext2_read_block(inode.i_block[0], b_buf);
        struct ext2_dir_entry* entry = (struct ext2_dir_entry*)(b_buf + offset);
        if (entry->rec_len == 0) break;
        
        if (entry->inode != 0) {
            int match = 1;
            for (int i = 0; i < entry->name_len; i++) {
                if (entry->name[i] != name[i]) {
                    match = 0;
                    break;
                }
            }
            if (match && name[entry->name_len] == '\0') {
                struct ext2_inode target_inode;
                ext2_read_inode(entry->inode, &target_inode);
                
                static struct vfs_node target_node;
                target_node.inode = entry->inode;
                target_node.mask = target_inode.i_mode;
                target_node.uid = target_inode.i_uid;
                target_node.gid = target_inode.i_gid;
                target_node.length = target_inode.i_size;
                target_node.flags = (target_inode.i_mode & 0x4000) ? VFS_DIRECTORY : VFS_FILE;
                target_node.read = ext2_node_read;
                target_node.write = ext2_node_write;
                target_node.readdir = ext2_node_readdir;
                target_node.finddir = ext2_node_finddir;
                return &target_node;
            }
        }
        offset += entry->rec_len;
    }
    return (struct vfs_node*)0;
}

struct vfs_node* ext2_mount(const char* source, const char* target) {
    uint8_t sector[1024];
    pata_read_sectors(2, 2, sector);
    struct ext2_super_block* psb = (struct ext2_super_block*)sector;
    if (psb->s_magic != 0xEF53) return (struct vfs_node*)0;
    
    sb.s_inodes_count = psb->s_inodes_count;
    sb.s_blocks_count = psb->s_blocks_count;
    sb.s_free_blocks_count = psb->s_free_blocks_count;
    sb.s_free_inodes_count = psb->s_free_inodes_count;
    sb.s_first_data_block = psb->s_first_data_block;
    sb.s_log_block_size = psb->s_log_block_size;
    sb.s_blocks_per_group = psb->s_blocks_per_group;
    sb.s_inodes_per_group = psb->s_inodes_per_group;
    sb.s_magic = psb->s_magic;
    
    block_size = 1024 << sb.s_log_block_size;
    
    uint32_t bg_sector = (block_size == 1024) ? 4 : 2;
    pata_read_sectors(bg_sector, 1, sector);
    struct ext2_group_desc* pbg = (struct ext2_group_desc*)sector;
    bg.bg_block_bitmap = pbg->bg_block_bitmap;
    bg.bg_inode_bitmap = pbg->bg_inode_bitmap;
    bg.bg_inode_table = pbg->bg_inode_table;
    bg.bg_free_blocks_count = pbg->bg_free_blocks_count;
    bg.bg_free_inodes_count = pbg->bg_free_inodes_count;
    
    static struct vfs_node root;
    root.inode = 2;
    root.flags = VFS_DIRECTORY;
    root.mask = 0755;
    root.read = ext2_node_read;
    root.write = ext2_node_write;
    root.readdir = ext2_node_readdir;
    root.finddir = ext2_node_finddir;
    return &root;
}

int ext2_unmount(struct vfs_node* mount_point) {
    return 0;
}

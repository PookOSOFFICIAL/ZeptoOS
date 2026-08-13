#include "tar.h"

#define MAX_TAR_FILES 32

struct tar_file_entry {
    char name[128];
    uint32_t address;
    uint32_t size;
    uint8_t type;
};

static struct tar_file_entry file_entries[MAX_TAR_FILES];
static int file_count = 0;
static struct vfs_node tar_nodes[MAX_TAR_FILES + 1];
static struct dirent tar_dirents[MAX_TAR_FILES + 1];

static uint32_t parse_octal(const char *val, int size) {
    uint32_t acc = 0;
    int i = 0;
    while (i < size && val[i] >= '0' && val[i] <= '7') {
        acc = acc * 8 + (val[i] - '0');
        i++;
    }
    return acc;
}

static uint32_t tar_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    uint32_t file_idx = node->inode;
    if (file_idx >= (uint32_t)file_count) return 0;
    uint32_t file_size = file_entries[file_idx].size;
    if (offset >= file_size) return 0;
    if (offset + size > file_size) {
        size = file_size - offset;
    }
    uint8_t* src = (uint8_t*)(file_entries[file_idx].address + offset);
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = src[i];
    }
    return size;
}

static uint32_t tar_write(struct vfs_node* node, uint32_t offset, uint32_t size, const uint8_t* buffer) {
    uint32_t file_idx = node->inode;
    if (file_idx >= (uint32_t)file_count) return 0;
    uint8_t* dest = (uint8_t*)(file_entries[file_idx].address + offset);
    for (uint32_t i = 0; i < size; i++) {
        dest[i] = buffer[i];
    }
    return size;
}

static struct dirent* tar_readdir(struct vfs_node* node, uint32_t index) {
    if (index >= (uint32_t)file_count) return (struct dirent*)0;
    return &tar_dirents[index];
}

static struct vfs_node* tar_finddir(struct vfs_node* node, const char* name) {
    for (int i = 0; i < file_count; i++) {
        const char* fname = file_entries[i].name;
        int match = 1;
        int k = 0;
        while (fname[k] != '\0' && name[k] != '\0') {
            if (fname[k] != name[k]) {
                match = 0;
                break;
            }
            k++;
        }
        if (match && fname[k] == '\0' && name[k] == '\0') {
            return &tar_nodes[i + 1];
        }
    }
    return (struct vfs_node*)0;
}

static int tar_mkdir(struct vfs_node* node, const char* name, uint16_t permission) {
    return 0;
}

static int tar_create(struct vfs_node* node, const char* name, uint16_t permission) {
    return 0;
}

static int tar_link(struct vfs_node* node, struct vfs_node* target, const char* name) {
    return 0;
}

static int tar_unlink(struct vfs_node* node, const char* name) {
    return 0;
}

struct vfs_node* tar_parse(uint32_t address) {
    file_count = 0;
    uint32_t current_address = address;

    while (1) {
        struct tar_header *header = (struct tar_header *)current_address;
        if (header->name[0] == '\0') break;

        uint32_t size = parse_octal(header->size, 11);
        
        if (file_count < MAX_TAR_FILES) {
            int k = 0;
            while (header->name[k] != '\0' && k < 127) {
                file_entries[file_count].name[k] = header->name[k];
                k++;
            }
            file_entries[file_count].name[k] = '\0';
            file_entries[file_count].address = current_address + 512;
            file_entries[file_count].size = size;
            file_entries[file_count].type = header->typeflag;
            file_count++;
        }

        current_address += ((size + 511) / 512 + 1) * 512;
    }

    tar_nodes[0].inode = 0;
    tar_nodes[0].flags = VFS_DIRECTORY;
    tar_nodes[0].mask = 0755;
    tar_nodes[0].read = 0;
    tar_nodes[0].write = 0;
    tar_nodes[0].readdir = tar_readdir;
    tar_nodes[0].finddir = tar_finddir;
    tar_nodes[0].mkdir = tar_mkdir;
    tar_nodes[0].create = tar_create;
    tar_nodes[0].link = tar_link;
    tar_nodes[0].unlink = tar_unlink;

    for (int i = 0; i < file_count; i++) {
        tar_nodes[i + 1].inode = i;
        tar_nodes[i + 1].flags = VFS_FILE;
        tar_nodes[i + 1].mask = 0644;
        tar_nodes[i + 1].length = file_entries[i].size;
        tar_nodes[i + 1].read = tar_read;
        tar_nodes[i + 1].write = tar_write;
        tar_nodes[i + 1].readdir = 0;
        tar_nodes[i + 1].finddir = 0;
        tar_nodes[i + 1].mkdir = tar_mkdir;
        tar_nodes[i + 1].create = tar_create;
        tar_nodes[i + 1].link = tar_link;
        tar_nodes[i + 1].unlink = tar_unlink;

        int k = 0;
        while (file_entries[i].name[k] != '\0' && k < 127) {
            tar_dirents[i].name[k] = file_entries[i].name[k];
            tar_dirents[i].name[k+1] = '\0';
            k++;
        }
        tar_dirents[i].ino = i + 1;
        tar_dirents[i].type = VFS_FILE;
    }

    return &tar_nodes[0];
}

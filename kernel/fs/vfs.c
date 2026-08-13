#include "vfs.h"

#define MAX_MOUNTS 8

struct mount_point {
    char path[128];
    struct vfs_node* node;
};

static struct vfs_node* root_node = (struct vfs_node*)0;
static struct mount_point mounts[MAX_MOUNTS];
static int mount_count = 0;

static int string_compare(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

static void string_copy(char* dest, const char* src) {
    while (*src) {
        *dest++ = *src++;
    }
    *dest = '\0';
}

void vfs_init() {
    mount_count = 0;
    root_node = (struct vfs_node*)0;
}

struct vfs_node* vfs_root() {
    return root_node;
}

uint32_t vfs_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    if (node && node->read) return node->read(node, offset, size, buffer);
    return 0;
}

uint32_t vfs_write(struct vfs_node* node, uint32_t offset, uint32_t size, const uint8_t* buffer) {
    if (node && node->write) return node->write(node, offset, size, buffer);
    return 0;
}

void vfs_open(struct vfs_node* node) {
    if (node && node->open) node->open(node);
}

void vfs_close(struct vfs_node* node) {
    if (node && node->close) node->close(node);
}

struct dirent* vfs_readdir(struct vfs_node* node, uint32_t index) {
    if (node && (node->flags & VFS_DIRECTORY) && node->readdir) {
        return node->readdir(node, index);
    }
    return (struct dirent*)0;
}

struct vfs_node* vfs_finddir(struct vfs_node* node, const char* name) {
    if (node && (node->flags & VFS_DIRECTORY) && node->finddir) {
        return node->finddir(node, name);
    }
    return (struct vfs_node*)0;
}

struct vfs_node* vfs_resolve_path(const char* path) {
    if (!path || path[0] != '/') return (struct vfs_node*)0;
    struct vfs_node* current = root_node;
    if (!current) return (struct vfs_node*)0;

    if (path[1] == '\0') return current;

    char token[128];
    int i = 1;
    while (path[i] != '\0') {
        int j = 0;
        while (path[i] != '\0' && path[i] != '/') {
            token[j++] = path[i++];
        }
        token[j] = '\0';
        if (path[i] == '/') i++;

        if (token[0] != '\0') {
            current = vfs_finddir(current, token);
            if (!current) return (struct vfs_node*)0;
        }
    }
    return current;
}

int vfs_mount(const char* path, struct vfs_node* local_root) {
    if (string_compare(path, "/") == 0) {
        root_node = local_root;
        mounts[0].node = local_root;
        string_copy(mounts[0].path, "/");
        if (mount_count == 0) mount_count = 1;
        return 0;
    }
    if (mount_count < MAX_MOUNTS) {
        string_copy(mounts[mount_count].path, path);
        mounts[mount_count].node = local_root;
        mount_count++;
        return 0;
    }
    return -1;
}

int vfs_umount(const char* path) {
    for (int i = 0; i < mount_count; i++) {
        if (string_compare(mounts[i].path, path) == 0) {
            for (int j = i; j < mount_count - 1; j++) {
                string_copy(mounts[j].path, mounts[j + 1].path);
                mounts[j].node = mounts[j + 1].node;
            }
            mount_count--;
            return 0;
        }
    }
    return -1;
}

int vfs_mkdir(const char* path, uint16_t permission) {
    struct vfs_node* parent = root_node;
    if (!parent || !parent->mkdir) return -1;
    return parent->mkdir(parent, path, permission);
}

int vfs_create(const char* path, uint16_t permission) {
    struct vfs_node* parent = root_node;
    if (!parent || !parent->create) return -1;
    return parent->create(parent, path, permission);
}

int vfs_link(const char* oldpath, const char* newpath) {
    struct vfs_node* old_node = vfs_resolve_path(oldpath);
    if (!old_node || !old_node->link) return -1;
    return old_node->link(old_node, old_node, newpath);
}

int vfs_unlink(const char* path) {
    struct vfs_node* node = vfs_resolve_path(path);
    if (!node || !node->unlink) return -1;
    return node->unlink(node, path);
}

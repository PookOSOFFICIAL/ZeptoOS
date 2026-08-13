#pragma once
#include "../lib/types.h"

#define VFS_FILE 1
#define VFS_DIRECTORY 2
#define VFS_SYMLINK 3

struct dirent {
    char name[128];
    uint32_t ino;
    uint8_t type;
};

struct vfs_node;

struct vfs_filesystem {
    char name[32];
    struct vfs_node* (*mount)(const char* source, const char* target);
    int (*unmount)(struct vfs_node* mount_point);
};

struct vfs_node {
    char name[128];
    uint32_t mask;
    uint32_t uid;
    uint32_t gid;
    uint32_t flags;
    uint32_t inode;
    uint32_t length;
    uint32_t impl;
    struct vfs_filesystem* fs;
    struct vfs_node* ptr;
    
    uint32_t (*read)(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
    uint32_t (*write)(struct vfs_node* node, uint32_t offset, uint32_t size, const uint8_t* buffer);
    void (*open)(struct vfs_node* node);
    void (*close)(struct vfs_node* node);
    struct dirent* (*readdir)(struct vfs_node* node, uint32_t index);
    struct vfs_node* (*finddir)(struct vfs_node* node, const char* name);
    int (*mkdir)(struct vfs_node* node, const char* name, uint16_t permission);
    int (*create)(struct vfs_node* node, const char* name, uint16_t permission);
    int (*link)(struct vfs_node* node, struct vfs_node* target, const char* name);
    int (*unlink)(struct vfs_node* node, const char* name);
};

void vfs_init();
struct vfs_node* vfs_root();
uint32_t vfs_read(struct vfs_node* node, uint32_t offset, uint32_t size, uint8_t* buffer);
uint32_t vfs_write(struct vfs_node* node, uint32_t offset, uint32_t size, const uint8_t* buffer);
void vfs_open(struct vfs_node* node);
void vfs_close(struct vfs_node* node);
struct dirent* vfs_readdir(struct vfs_node* node, uint32_t index);
struct vfs_node* vfs_finddir(struct vfs_node* node, const char* name);
int vfs_mount(const char* path, struct vfs_node* local_root);
int vfs_umount(const char* path);
struct vfs_node* vfs_resolve_path(const char* path);
int vfs_mkdir(const char* path, uint16_t permission);
int vfs_create(const char* path, uint16_t permission);
int vfs_link(const char* oldpath, const char* newpath);
int vfs_unlink(const char* path);

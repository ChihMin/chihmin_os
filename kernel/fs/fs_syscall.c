/* This file use for NCTU OSDI course */

// It's handel the file system APIs 
#include <inc/stdio.h>
#include <inc/syscall.h>
#include <fs.h>
#include <kernel/mem.h>
#include <kernel/task.h>

/*TODO: Lab7, file I/O system call interface.*/
// Below is POSIX like I/O system call
extern struct fs_fd fd_table[];
extern struct fs_dev fat_fs; 
extern Task * cur_task;

int errno(int error) {
    int ret_val = error;
    switch (error) {
    case -4:
        ret_val = -STATUS_ENOENT;
        break;
    
    case -8:
        ret_val = -STATUS_EEXIST;
        break; 
    }
    return ret_val;
}

int check_valid_fd(struct fs_fd *fd) {
    bool is_valid = false;
    int i;
    for (i = 0; i < FS_FD_MAX; ++i)
        if (fd == &fd_table[i]) {
            is_valid = true;
            break;
        }
    return is_valid;
}

int sys_open(const char *file, int flags, int mode)
{
    //We dont care the mode.
    int i;
    int empty_index = -1;
    int find_file = false;
    struct fs_fd *cur_fd = -1;
    int new_flags;

    for (i = 0; i < FS_FD_MAX; ++i) {
        cur_fd = &fd_table[i];
        if (!strcmp(cur_fd->path, file)) {
            find_file = true;
            break;
        } 
        else if (!find_file && !(~empty_index) && !strcmp("", cur_fd->path)) {
            empty_index = i;
        }
    }
    
    int ret_val = -1;
    if (find_file) { 
        cur_fd->flags = flags;
        ret_val = cur_fd->fs->ops->open(cur_fd);
    } else if (~empty_index) {
        cur_fd = &fd_table[empty_index];
        cur_fd->flags = flags;
        strcpy(cur_fd->path, file);
        ret_val = cur_fd->fs->ops->open(cur_fd);
    }
    if (ret_val < 0)
        return errno(ret_val);
    return PADDR(cur_fd); // return fs_fd pointer 
}

int sys_close(int fd)
{
    int ret_val;
    struct fs_fd *cur_fd = (struct fs_fd *) KADDR(fd);
    if (!check_valid_fd(cur_fd)) {
        return -STATUS_EINVAL;
    }

    ret_val = cur_fd->fs->ops->close(cur_fd);
    if (ret_val == 0)
        strcpy(cur_fd->path, "");
    printk("[%s] ret = %d\n", __func__, ret_val);
    return ret_val; 
}
int sys_read(int fd, void *buf, size_t len)
{
    int ret_val;
    struct fs_fd *cur_fd = (struct fs_fd *) KADDR(fd);

    struct PageInfo* page = page_lookup(cur_task->pgdir, buf, NULL);
    if (page == NULL)
        return -STATUS_EINVAL;
    else if (!check_valid_fd(cur_fd))
        return -STATUS_EBADF;

    ret_val = cur_fd->fs->ops->read(cur_fd, buf, len);
    printk("[SYS READ] ret = 0x%x\n", ret_val);
    
    return ret_val;
}


int sys_write(int fd, const void *buf, size_t len)
{
    int ret_val;
    struct fs_fd *cur_fd = (struct fs_fd *) KADDR(fd);

    struct PageInfo* page = page_lookup(cur_task->pgdir, buf, NULL);
    if (page == NULL)
        return -STATUS_EINVAL;
    else if (!check_valid_fd(cur_fd))
        return -STATUS_EBADF;
   
    ret_val = cur_fd->fs->ops->write(cur_fd, buf, len);
    
    return ret_val;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
/* TODO */
    int ret_val;
    off_t top_offset = 0;
    struct fs_fd *cur_fd = (struct fs_fd *) KADDR(fd);
    
    switch (whence) {
    case SEEK_END:
        top_offset = cur_fd->size + offset;
        break; 

    case SEEK_SET:
        top_offset = offset;
        break;

    case SEEK_CUR:
        top_offset = cur_fd->pos + offset;
        break;
    }

    ret_val = cur_fd->fs->ops->lseek(cur_fd, top_offset);
    if (ret_val != 0)
        return ret_val;
    cur_fd->pos = top_offset;
    return offset;
}

int sys_unlink(const char *pathname)
{
/* TODO */ 
}


              


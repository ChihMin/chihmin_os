/* This file use for NCTU OSDI course */

// It's handel the file system APIs 
#include <inc/stdio.h>
#include <inc/syscall.h>
#include <fs.h>
#include <kernel/mem.h>

/*TODO: Lab7, file I/O system call interface.*/
// Below is POSIX like I/O system call
extern struct fs_fd fd_table[];
extern struct fs_dev fat_fs; 
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
    return PADDR(cur_fd); // return fs_fd pointer 
}

int sys_close(int fd)
{
    int ret_val;
    struct fs_fd *cur_fd = (struct fs_fd *) KADDR(fd);
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
    ret_val = cur_fd->fs->ops->read(cur_fd, buf, len);
    
    return ret_val;
}
int sys_write(int fd, const void *buf, size_t len)
{
    int ret_val;
    struct fs_fd *cur_fd = (struct fs_fd *) KADDR(fd);
    ret_val = cur_fd->fs->ops->write(cur_fd, buf, len);
    
    return ret_val;
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
/* TODO */
}

int sys_unlink(const char *pathname)
{
/* TODO */ 
}


              


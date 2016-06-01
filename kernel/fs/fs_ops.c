/* This file use for NCTU OSDI course */
/* It's contants fat file system operators */

#include <inc/stdio.h>
#include <fs.h>
#include <fat/ff.h>
#include <diskio.h>

extern struct fs_dev fat_fs;

/*TODO: Lab7, fat level file operator.*/

int fat_mount(struct fs_dev *fs, unsigned long rwflag, const void* data)
{

}
int fat_mkfs(const char* device_name)
{

}
int fat_open(struct fs_fd* file)
{
}
int fat_close(struct fs_fd* file)
{

}
int fat_read(struct fs_fd* file, void* buf, size_t count)
{

}
int fat_write(struct fs_fd* file, const void* buf, size_t count)
{
}
int fat_lseek(struct fs_fd* file, off_t offset)
{
}
int fat_unlink(struct fs_fd* file, const char *pathname)
{
}

struct fs_ops elmfat_ops = {
    .dev_name = "elmfat",
    .mount = fat_mount,
    .mkfs = fat_mkfs,
    .open = fat_open,
    .close = fat_close,
    .read = fat_read,
    .write = fat_write,
    .lseek = fat_lseek
};




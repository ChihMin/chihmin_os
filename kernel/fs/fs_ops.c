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
    int res;
    printk("[MKFS] Device : %s\n", device_name);
    res = f_mkfs("/", 0, 0);

    if (res != 0)
        return -1;
    return 0;
}

int fat_open(struct fs_fd* file)
{
    FIL* data = file->data;
    uint32_t flags = 0;
    
    flags |= O_WRONLY & file->flags ?  FA_WRITE : 0;
    flags |= O_CREAT & file->flags ? FA_CREATE_NEW : 0;
    flags |= file->flags == 0 ? FA_READ : 0;
    flags |= O_RDWR & file->flags ? FA_READ|FA_WRITE : 0; 

    int ret_val = f_open(data, file->path, flags);
    file->pos = 0;
    return ret_val; 
}

int fat_close(struct fs_fd* file)
{
   int ret_val;
   ret_val = f_close(file->data);
   return ret_val; 
}

int fat_read(struct fs_fd* file, void* buf, size_t count)
{   
    FIL* data = file->data;
    UINT bw;
    int ret_val = -1;
    ret_val = f_read(data, buf, count, &bw);
    if (ret_val != 0)
        return -1;
    file->pos += bw;
    return bw;
}

int fat_write(struct fs_fd* file, const void* buf, size_t count)
{
    FIL* data = file->data;
    UINT bw;
    int ret_val = -1;
    ret_val = f_write(data, buf, count, &bw);
    if (ret_val != 0)
        return -1;
    
    int next_pos = file->pos + bw;
    int size_offset = 0;
    if (next_pos > file->size) 
        size_offset = next_pos - file->size;
    file->size += size_offset;   // update size of file
    file->pos = next_pos;
    
    return bw;
}

int fat_lseek(struct fs_fd* file, off_t offset)
{
    FIL* data = file->data;
    int ret_val = -1;
    ret_val = f_lseek(data, offset);
    
    return ret_val;
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




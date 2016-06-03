/* This file use for NCTU OSDI course */
/* It's contants file operator's wapper API */
#include <fs.h>
#include <fat/ff.h>
#include <inc/string.h>
#include <inc/stdio.h>

/* Static file objects */
FIL file_objs[FS_FD_MAX];

/* Static file system object */
FATFS fat;

struct fs_fd fd_table[FS_FD_MAX];

extern struct fs_ops elmfat_ops; //We only one file system...
struct fs_dev fat_fs = {
    .dev_id = 1, //In this lab we only use second IDE disk
    .path = {0}, // Not yet mount to any path
    .ops = &elmfat_ops,
    .data = &fat
};
    
/*TODO: Lab7, VFS level file API.*/
int fs_init()
{
    int res, i;
    
    /* Initial fd_tables */
    for (i = 0; i < FS_FD_MAX; i++)
    {
        fd_table[i].flags = 0;
        fd_table[i].size = 0;
        fd_table[i].pos = 0;
        fd_table[i].type = 0;
        fd_table[i].ref_count = 0;
        fd_table[i].data = &file_objs[i];
        fd_table[i].fs = &fat_fs;
    }
    
    /* Mount fat file system at "/" */
    /* Check need mkfs or not */
    if ((res = fs_mount("elmfat", "/", 0, NULL)) != 0)
    {
        fat_fs.ops->mkfs("elmfat");
        fs_mount("elmfat", "/", 0, NULL);
    }
       
}
int fs_mount(const char* device_name, const char* path, unsigned long rwflag, const void* data)
{
    return -STATUS_EIO;
} 

int file_read(struct fs_fd* fd, void *buf, size_t len)
{

}

int file_write(struct fs_fd* fd, const void *buf, size_t len)
{

}

int file_open(struct fs_fd* fd, const char *path, int flags)
{

}

int file_close(struct fs_fd* fd)
{

}
int file_lseek(struct fs_fd* fd, off_t offset)
{

}
int file_unlink(const char *path)
{
}


/**
 * @ingroup Fd
 * This function will allocate a file descriptor.
 *
 * @return -1 on failed or the allocated file descriptor.
 */
int fd_new(void)
{
	struct fs_fd* d;
	int idx;

	/* find an empty fd entry */
	for (idx = 0; idx < FS_FD_MAX && fd_table[idx].ref_count > 0; idx++);


	/* can't find an empty fd entry */
	if (idx == FS_FD_MAX)
	{
		idx = -1;
		goto __result;
	}

	d = &(fd_table[idx]);
	d->ref_count = 1;

__result:
	return idx;
}

/**
 * @ingroup Fd
 *
 * This function will return a file descriptor structure according to file
 * descriptor.
 *
 * @return NULL on on this file descriptor or the file descriptor structure
 * pointer.
 */
struct fs_fd* fd_get(int fd)
{
	struct fs_fd* d;

	if ( fd < 0 || fd > FS_FD_MAX ) return NULL;

	d = &fd_table[fd];

	/* increase the reference count */
	d->ref_count ++;

	return d;
}

/**
 * @ingroup Fd
 *
 * This function will put the file descriptor.
 */
void fd_put(struct fs_fd* fd)
{

	fd->ref_count --;

	/* clear this fd entry */
	if ( fd->ref_count == 0 )
	{
		//memset(fd, 0, sizeof(struct fs_fd));
		memset(fd->data, 0, sizeof(FIL));
	}
};



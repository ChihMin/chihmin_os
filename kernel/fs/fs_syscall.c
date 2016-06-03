/* This file use for NCTU OSDI course */

// It's handel the file system APIs 
#include <inc/stdio.h>
#include <inc/syscall.h>
#include <fs.h>

/*TODO: Lab7, file I/O system call interface.*/
// Below is POSIX like I/O system call 
int sys_open(const char *file, int flags, int mode)
{
    //We dont care the mode.
/* TODO */
}

int sys_close(int fd)
{
/* TODO */
}
int sys_read(int fd, void *buf, size_t len)
{
/* TODO */
}
int sys_write(int fd, const void *buf, size_t len)
{
/* TODO */
}

off_t sys_lseek(int fd, off_t offset, int whence)
{
/* TODO */
}

int sys_unlink(const char *pathname)
{
/* TODO */ 
}


              


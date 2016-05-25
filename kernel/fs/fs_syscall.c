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

}

int sys_close(int fd)
{

}
int sys_read(int fd, void *buf, size_t len)
{

}
int sys_write(int fd, const void *buf, size_t len)
{

}

off_t sys_lseek(int fd, off_t offset, int whence)
{

}

int sys_unlink(const char *pathname)
{
 
}


              


/* This file use for NCTU OSDI course */
#include <fs.h>
#include <fat/diskio.h>
#include <fat/ff.h>
#include <kernel/drv/disk.h>

#define DISK_ID 1

/*TODO: Lab7, low level file operator.*/
DSTATUS disk_initialize (BYTE pdrv)
{
    //printk("disk_initialize %d\n", pdrv);
    /* TODO */
    return 0;
}
DSTATUS disk_status (BYTE pdrv)
{
    //printk("disk_status\n");
    /* TODO */
    return 0;
}
DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    int err = 0;
    int i = count;
    BYTE *ptr = buff;
    UINT cur_sector = sector;
    
    //printk("disk_read %d sector %d count %d\n", pdrv, sector, count);
    
    /* TODO */
}
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    int err = 0;
    int i = count;
    BYTE *ptr = buff;
    UINT cur_sector = sector;
    
    //printk("disk_write %d sector %d count %d\n", pdrv, sector, count);
    /* TODO */    

}
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
    uint32_t *retVal = (uint32_t *)buff;
    //printk("disk_ioctl drv=%d cmd=%d\n", pdrv, cmd);
    /* TODO */    
}

DWORD get_fattime (void)
{
    /* TODO */
    return 0;
}

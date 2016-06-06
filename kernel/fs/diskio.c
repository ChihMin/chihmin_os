/* This file use for NCTU OSDI course */
#include <fs.h>
#include <fat/diskio.h>
#include <fat/ff.h>
#include <kernel/drv/disk.h>

#define DISK_ID 1

/*TODO: Lab7, low level file operator.*/

/**
  * @brief  Initial IDE disk
  * @param  pdrv: disk type
  * @retval disk error status
  *         - RES_OK: Initial success
  *         - STA_NOINIT: Intial failed
  */
DSTATUS disk_initialize (BYTE pdrv)
{
    printk("[disk_initialize] %d\n", pdrv);
     
    /* TODO */
    disk_init();
    return 0;
}

/**
  * @brief  Get disk current status
  * @param  pdrv: disk type
  * @retval disk status
  *         - RES_OK: Normal status
  *         - STA_NODISK: ...
  */
DSTATUS disk_status (BYTE pdrv)
{
    printk("[disk_status]\n");
    /* TODO */
    return 0;
}

/**
  * @brief  Read serval sector form a IDE disk
  * @param  pdrv: disk type
  * @param  buff: destination memory start address
  * @param  sector: start sector number
  * @param  count: number of sector
  * @retval disk status (See diskio.h)
  *         - RES_OK: success
  *         - < 0: failed
  */
DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count)
{
    int err = 0;
    int i = count;
    BYTE *ptr = buff;
    UINT cur_sector = sector;
    
    /* TODO */
    printk("[DISK_READ]  %d sector %d count %d\n", pdrv, sector, count);
    return ide_read_sectors(DISK_ID, i, cur_sector, ptr);  
}

/**
  * @brief  Write serval sector to a IDE disk
  * @param  pdrv: disk type
  * @param  buff: memory start address
  * @param  sector: destination start sector number
  * @param  count: number of sector
  * @retval disk status (See diskio.h)
  *         - RES_OK: success
  *         - < 0: failed
  */
DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count)
{
    int err = 0;
    int i = count;
    BYTE *ptr = buff;
    UINT cur_sector = sector;
    
    //printk("[DISK_WRITE]  %d sector %d count %d\n", pdrv, sector, count);
    /* TODO */    
    return ide_write_sectors(DISK_ID, i, sector, ptr);  
}

/**
  * @brief  Get disk information form disk
  * @param  pdrv: disk type
  * @param  cmd: disk control command (See diskio.h)
  *         - GET_SECTOR_COUNT
  *         - GET_BLOCK_SIZE (Same as sector size)
  * @param  buff: return memory space
  * @retval disk status (See diskio.h)
  *         - RES_OK: success
  *         - < 0: failed
  */
DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff)
{
    uint32_t *retVal = (uint32_t *)buff;
    printk("[DISK IOCTL] drv=%d cmd=%d\n", pdrv, cmd);
    /* TODO */
     
    switch (cmd) {
    case GET_SECTOR_COUNT:
        *retVal = 32 * (1<<20) / 512;
        break; 
    }
    return 0;
}


/**
  * @brief  Get OS timestamp
  * @retval tick of CPU
  */
DWORD get_fattime (void)
{
    /* TODO */
    return 0;
}

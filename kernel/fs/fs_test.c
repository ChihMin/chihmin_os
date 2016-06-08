#include "ff.h"

/* Low level I/O API test */
void fs_test()
{
    FATFS fs;      /* File system object (volume work area) */
    FIL fil;       /* File object */
    FRESULT res;   /* API result code */
    UINT bw;       /* Bytes written */

    /* Register work area (do not care about error) */
    res = f_mount(&fs, "/", 1);
    if (res != 0)
    {
        printk("Mount failed %d, try do mkfs.\n", res);
        /* Create FAT volume with default cluster size */
        res = f_mkfs("/", 0, 0);
        res = f_mount(&fs, "/", 1); //Remount
    }

    /* Create a file as new */
    res = f_mkdir("tmp");
    if (res != 0)
        printk("ERROR Make a new folder ~~~~~~~\n");
    res = f_open(&fil, "tmp/hello.txt", FA_CREATE_ALWAYS | FA_WRITE);
    
    if (res != 0)
        printk("Open failed %d\n", res);

    /* Write a message */
    f_write(&fil, "Hello, World!\r\n", 15, &bw);

    /* Close the file */
    f_close(&fil);
    
    FIL tmp;
    f_open(&tmp, "tmp/shit.txt", FA_CREATE_ALWAYS|FA_WRITE);
    f_write(&tmp, "Hello, World!\r\n", 10, &bw);
    f_close(&tmp);
    
    res = f_mkdir("tmp/hahaha");
     

    DIR dir;
    FILINFO fno;
    printk("ls folder = tmp\n");
    f_opendir(&dir, "tmp");
    res = f_readdir(&dir, &fno);
    while (strlen(fno.fname)) {
        printk("[%s] ret = %d, filename = %s, fsize = %d, date = %d, time = %d\n", __func__, res, fno.fname, fno.fsize, fno.fdate, fno.ftime);
        res = f_readdir(&dir, &fno);
    }
    f_closedir(&dir);
    /* Unregister work area */
    f_mount(0, "", 0);
}

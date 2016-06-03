#include <inc/syscall.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/shell.h>

char hist[SHELL_HIST_MAX][BUF_LEN];

int hist_head;
int hist_tail;
int hist_curr;

/*  Prototypes  */
int mon_help(int argc, char **argv);
int mem_stat(int argc, char **argv);
int print_tick(int argc, char **argv);
int chgcolor(int argc, char **argv);
int forktest(int argc, char **argv);
int print_pid(int argc, char **argv);
int fork_single(int argc, char **argv);
int kill_pid(int argc, char **argv);
int filetest(int argc, char **argv);
int fs_seek_test(int argc, char **argv);
int fs_speed_test(int argc, char **argv);

struct Command commands[] = {
  { "help", "Display this list of commands", mon_help },
  { "mem_stat", "Show current usage of physical memory", mem_stat },
  { "print_tick", "Display system tick", print_tick },
  { "chgcolor", "Change screen text color", chgcolor },
  { "forktest", "Test functionality of fork()", forktest },
  { "print_pid", "Get current pid number", print_pid },
  { "fork_single", "Fork only one process", fork_single},
  { "kill_pid", "Kill specific process id", kill_pid},
  { "filetest", "Test create file", filetest },
  { "fs_seek_test", "Test seek file", fs_seek_test },
  { "fs_speed_test", "Test R/W speed", fs_speed_test}
};
const int NCOMMANDS = (sizeof(commands)/sizeof(commands[0]));

int print_pid(int argc, char **argv) {
    cprintf("Current pid is %d\n", getpid());
    return 0;
}

int mem_stat(int argc, char **argv)
{
  cprintf("%-10s MEM_STAT %10s\n", "--------", "--------");
  cprintf("Used: %18d pages\n", get_num_used_page());
  cprintf("Free: %18d pages\n", get_num_free_page());
  return 0;
}

int mon_help(int argc, char **argv)
{
  int i;

  for (i = 0; i < NCOMMANDS; i++)
    cprintf("%s - %s\n", commands[i].name, commands[i].desc);
  return 0;
}

int print_tick(int argc, char **argv)
{
  cprintf("Now tick = %d\n", get_ticks());
  return 0;
}

int chgcolor(int argc, char **argv)
{
  if (argc > 1)
  {
    char fore = argv[1][0] - '0';
    settextcolor(fore, 0);
    cprintf("Change color %d!\n", fore);
  }
  else
  {
    cprintf("No input text color!\n");
  }
  return 0;
}


#define WHITESPACE "\t\r\n "
#define MAXARGS 16

static int runcmd(char *buf)
{
  int argc;
  char *argv[MAXARGS];
  int i;

  // Parse the command buffer into whitespace-separated arguments
  argc = 0;
  argv[argc] = 0;
  while (1) {
    // gobble whitespace
    while (*buf && strchr(WHITESPACE, *buf))
      *buf++ = 0;
    if (*buf == 0)
      break;

    // save and scan past next arg
    if (argc == MAXARGS-1) {
      cprintf("Too many arguments (max %d)\n", MAXARGS);
      return 0;
    }
    argv[argc++] = buf;
    while (*buf && !strchr(WHITESPACE, *buf))
      buf++;
  }
  argv[argc] = 0;

  // Lookup and invoke the command
  if (argc == 0)
    return 0;
  for (i = 0; i < NCOMMANDS; i++) {
    if (strcmp(argv[0], commands[i].name) == 0)
      return commands[i].func(argc, argv);
  }
  cprintf("Unknown command '%s'\n", argv[0]);
  return 0;
}



void task_job()
{
	int pid = 0, parent_id = 0;
	int i;

	pid = getpid();
    parent_id = parent();
	for (i = 0; i < 10; i++)
	{
		cprintf("Im %d, parent = %d, now=%d\n", pid, parent_id, i);
		sleep(100);
	}
}

int forktest(int argc, char **argv)
{
  /* Below code is running on user mode */
  if (!fork())
  {

    /*Child*/
    task_job();
    if (fork())
      task_job();
    else
    {
      if (fork())
        task_job();
      else
        if (fork())
          task_job();
        else
          task_job();
    }
  }
  /* task recycle */
  kill_self();
  return 0;
}
#define BUFSIZE 30
int filetest(int argc, char **argv)
{
    int fd = -1;
    int ret;
    char *test_str = "This is the last LAB!! Yah!";
    char buf[BUFSIZE] = {0};
    
    if ((fd = open("hello.txt", O_WRONLY | O_CREAT | O_TRUNC, 0)) >= 0)
    {
        cprintf("Open successed!\n");
        
        ret = write(fd, test_str, strlen(test_str));
        if (ret > 0)
            cprintf("Write %d bytes!\n", ret);
        else
            cprintf("Write failed %d!\n", ret);
            
        close(fd);
    }
    else
        cprintf("Open failed!\n");
        
    if ((fd = open("hello.txt", O_RDONLY, 0)) >= 0)
    {
        cprintf("Open successed!\n");
        
        ret = read(fd, buf, BUFSIZE);
        if (ret > 0)
            cprintf("Read \"%s\"\n", buf);
        else
            cprintf("Read failed %d!\n", ret);
            
        close(fd);
    }
    else
        cprintf("Open failed!\n");
            
    return 0;
}

int fs_seek_test(int argc, char **argv)
{
    int i;
    off_t offset;
    int fd = -1;
    int ret;
    char buf[BUFSIZE] = {0};
    
    for (i = 0; i < BUFSIZE; i++)
    {
        buf[i] = i;
    }
    if ((fd = open("test2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0)) >= 0)
    {
        ret = write(fd, buf, 10); // write test pattern
        if (ret == 10)
           cprintf("Write test pattern successed!\n"); 
        close(fd);
    }
    
    if ((fd = open("test2.txt", O_RDWR, 0)) >= 0)
    {
        offset = lseek(fd, 10, SEEK_END); //seek to file end + 10 bytes
        cprintf("File offset =%d\n");
        ret = write(fd, &(buf[10]), 10); 
        offset = lseek(fd, 0, SEEK_SET); //seek to file begin
        
        for (i = 0; i < BUFSIZE; i++)
        {
            buf[i] = 0;
        }
        ret = read(fd, buf, BUFSIZE);
        
        if (ret >= 0)
        {
            cprintf("Read total %d bytes\n", ret);
            for (i = 0; i < ret; i++)
            {
                cprintf("%d ", buf[i]);
            }
            cprintf("\n");
        }
        close(fd);
    }
    else
        cprintf("Open failed!\n");
}
#define fsrw_fn                   "/test.dat"
#define fsrw_data_len             180              /* Less than 256 */
#define FS_TEST_TIMES       5000
int fs_speed_test(int argc, char **argv)
{
    int fd;
    int stop_flag = 0;
    int index,length;
    uint32_t round;
    uint32_t tick_start,tick_end,read_speed,write_speed;
    

    static uint8_t write_data[fsrw_data_len];
    static uint8_t read_data[fsrw_data_len];

    round = 0;

    while(round < 5)
    {

        /* creat file */
        fd = open(fsrw_fn, O_WRONLY | O_CREAT | O_TRUNC, 0);
        if (fd < 0)
        {
            cprintf("fsrw open file for write failed\n");
            return;
        }

        /* plan write data */
        for (index = 0; index < fsrw_data_len; index ++)
        {
            write_data[index] = index;
        }

        /* write N times */
        tick_start = get_ticks();
        for(index=0; index<FS_TEST_TIMES ; index++)
        {
            length = write(fd, write_data, fsrw_data_len);
            if (length != fsrw_data_len)
            {
                cprintf("fsrw write data failed\n");
                close(fd);
                return;
            }
        }
        tick_end = get_ticks();
        write_speed = fsrw_data_len*FS_TEST_TIMES *100/(tick_end-tick_start);

        /* close file */
        close(fd);

        /* open file read only */
        fd = open(fsrw_fn, O_RDONLY, 0);
        if (fd < 0)
        {
            cprintf("fsrw open file for read failed\n");
            return;
        }

        /* verify data */
        tick_start = get_ticks();
        for(index=0; index<FS_TEST_TIMES ; index++)
        {
            uint32_t i;

            length = read(fd, read_data, fsrw_data_len);
            if (length != fsrw_data_len)
            {
                cprintf("fsrw read file failed\r\n");
                close(fd);
                return;
            }
            for(i=0; i<fsrw_data_len; i++)
            {
                if( read_data[i] != write_data[i] )
                {
                    cprintf("fsrw data error!\r\n");
                    close(fd);
                    return;
                }
            }
        }
        tick_end = get_ticks();
        read_speed = fsrw_data_len*FS_TEST_TIMES *100/(tick_end-tick_start);

        cprintf("thread fsrw round %d ",round++);
        cprintf("rd:%dbyte/s,wr:%dbyte/s\r\n",read_speed,write_speed);

        /* close file */
        close(fd);
    }
}

int fork_single(int argc, char **argv) {
    int pid = fork();
    //cprintf("Current task[%d]: fork child ID = %d\n", getpid(), pid);
    task_job();
    kill_self();
        
    return 0;
}

int kill_pid(int argc, char **argv) {
    if (argc < 2) {
        cprintf("Please input correct format : kill_pid ${pid}");
        return 0;
    }

    int sum = 0;
    int i;
    int len = strlen(argv[1]);
    for (i = 0; i < len; ++i)
        sum = sum * 10 + argv[1][i] - '0';
    cprintf("[kill pid] argc = %d, argv =  %d\n", argc, sum);
    kill(sum);
    return 0; 
}
int fork_context() {
    int pid = fork();
    cprintf("Current task[%d]: fork child ID = %d\n", getpid(), pid);
    return 0;
}

void shell()
{
  char *buf;
  hist_head = 0;
  hist_tail = 0;
  hist_curr = 0;

  cprintf("Welcome to the OSDI course!\n");
  cprintf("Type 'help' for a list of commands.\n");
    
  while(1)
  {
    buf = readline("OSDI> ");
    if (buf != NULL)
    {
      strcpy(hist[hist_tail], buf);
      hist_tail = (hist_tail + 1) % SHELL_HIST_MAX;
      if (hist_head == hist_tail)
      {
        hist_head = (hist_head + 1) % SHELL_HIST_MAX;
        hist[hist_tail][0] = 0;
      }
      hist_curr = hist_tail;

      if (runcmd(buf) < 0)
        break;
    }
  }
}

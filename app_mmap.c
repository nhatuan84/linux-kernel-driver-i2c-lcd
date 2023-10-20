#include <stdio.h>      // printf
#include <fcntl.h>      // open
#include <unistd.h>     // read, close, getpagesize
#include <sys/mman.h>   // mmap
#include <string.h>     // memcmp, strlen
#include <assert.h>     // assert

#define DEMO_DEV_NAME   "/dev/lcd_dev"

int main()
{
    int fd;
    char *addr = NULL;
    int ret;
    char buf[32] = {0};
    char *message = "0123456789abcdefghijklmnopqrstuvwxyz\n";

    fd = open(DEMO_DEV_NAME, O_RDWR);
    if (fd < 0) 
    {
        printf("open device %s failed\n", DEMO_DEV_NAME);
        return -1;
    }
    //offset should be multiple of page_size
    addr = mmap(NULL, (size_t)getpagesize(), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_LOCKED, fd, 1*(size_t)getpagesize());
    //need to munmap to release device
    munmap(addr, (size_t)getpagesize());
    close(fd);

    /////////////////////
    fd = open(DEMO_DEV_NAME, O_RDWR);
    addr = mmap(NULL, (size_t)getpagesize(), PROT_READ | PROT_WRITE,
                MAP_SHARED | MAP_LOCKED, fd, 1*(size_t)getpagesize());
    
    ret = sprintf(addr, "%s", message);

    lseek(fd, 3, SEEK_SET);

    for(int i=0; i<10; i++)
    {
        
        ret = read(fd, buf, 2);
        printf("%s\n", buf);
    }
    printf("\n");

    munmap(addr, (size_t)getpagesize());
    close(fd);

    return 0;
}

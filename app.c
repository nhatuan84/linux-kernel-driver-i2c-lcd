/***************************************************************************//**
*  \file       test_app.c
*
*  \details    Userspace application to test the Device driver
*
*  \author     EmbeTronicX
*
*  \Tested with Linux raspberrypi 5.10.27-v7l-embetronicx-custom+
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "ioctl_cmd.h"

int main()
{
        int fd;
        ioctl_cmd_t ioctl_cmd;

        printf("\nOpening Driver\n");
        fd = open("/dev/lcd_dev", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
 
        printf("backlight\n");
        scanf("%d",&ioctl_cmd.backlight);
        printf("home\n");
        scanf("%d",&ioctl_cmd.home);
        printf("clear\n");
        scanf("%d",&ioctl_cmd.clear);

        ioctl(fd, IOCTL_WRITE, (ioctl_cmd_t*) &ioctl_cmd); 
 
        printf("Reading from Driver\n");
        ioctl(fd, IOCTL_READ, (ioctl_cmd_t*) &ioctl_cmd);
        printf("backlight %d\n", ioctl_cmd.backlight);
        printf("home %d\n", ioctl_cmd.home);
        printf("clear %d\n", ioctl_cmd.clear);
        
        write (fd, "012", strlen("012"));
        write (fd, "34", strlen("34"));
        write (fd, "56", strlen("56"));
        write (fd, "789", strlen("789"));
        write (fd, "abc", strlen("abc"));
        write (fd, "def", strlen("def"));
        write (fd, "ghi", strlen("ghi"));
        write (fd, "klm", strlen("klm"));

        char buf[33];
        memset(buf, 0, 33);
        read (fd, buf, 33);
        printf("read: %s\n", buf);

        printf("Closing Driver\n");
        close(fd);

        //
        char buffer[20];

        for (int i=0; i<12; i++)
        {
                fd = open("/dev/lcd_dev", O_RDWR);
                int j = snprintf(buffer, 20, "tuan-%d", i);
                write (fd, buffer, strlen(buffer));
                close(fd);
        }

}
#ifndef IOCTL_CMD_H
#define IOCTL_CMD_H
#include <linux/ioctl.h>
 
typedef struct
{
    int backlight;
    int home;
    int clear;
} ioctl_cmd_t;
 
#define IOCTL_WRITE     _IOW('l', 1, ioctl_cmd_t*)
#define IOCTL_READ     _IOR('l', 2, ioctl_cmd_t*)
 
#endif
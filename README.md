# linux-kernel-driver-i2c-lcd
set the backlight, home, clear via /sys/lcd_dev/

set the backlight, home, clear via ioctl /dev/lcd_dev

send/receive text to display via /dev/lcd_dev

update the bus and device I2C address:

#define I2C_BUS_NR              1     

#define I2C_DEV_ADDR            0x27 

build:
run make
sudo insmod lcd_driver
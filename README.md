# linux-kernel-driver-i2c-lcd
set the backlight, home, clear via /sys/lcd_dev/

set the backlight, home, clear via ioctl /dev/lcd_dev

send/receive text to display via /dev/lcd_dev


build:
run make
sudo insmod lcd_driver

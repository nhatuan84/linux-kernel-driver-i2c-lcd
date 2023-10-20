obj-m += lcd_driver.o

lcd_driver-objs := i2c_lcd_driver_mmap.o queue.o LiquidCrystal_I2C.o 


KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	gcc -o app1 app.c
	gcc -o app2 app_mmap.c
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
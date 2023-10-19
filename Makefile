obj-m += lcd_driver.o

lcd_driver-objs := i2c_lcd_driver.o queue.o LiquidCrystal_I2C.o 


KDIR = /lib/modules/$(shell uname -r)/build
 
 
all:
	make -C $(KDIR)  M=$(shell pwd) modules
	gcc -o app app.c
clean:
	make -C $(KDIR)  M=$(shell pwd) clean
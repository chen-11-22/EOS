obj-m += final_driver.o
KDIR = /home/jason/linux
all:
		make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules	
clean:
		make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) clean

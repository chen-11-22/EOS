obj-m += final_driver.o
KDIR = /home/chen/linux
all:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(shell pwd) modules
	gcc -o final_client final_client.c
clean:
	make ARCH=arm64 CROSS_COMPILE=aarch64-linux-gnu- -C $(KDIR) M=$(PWD) clean
	rm final_client.c

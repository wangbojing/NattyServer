

PWD = $(shell pwd)
KERNEL_SRC = /home/wangbojing/share/linux-4.4.8/

obj-m := DaveQueue.o
module-objs := DaveQueue.o

all:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules

install:
	insmod DaveQueue.ko
	mknod /dev/DaveMqDev c 247 0

clean:
	rm *.ko
	rm *.o




obj-m += write_device.o
obj-m += read_device.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules


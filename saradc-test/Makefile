CROSS_COMPILE := aarch64-linux-gnu-
CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld

KERNELDIR := /home/dar/rk3399/rk3399_linux_release_v2.5.1_20210301_split_dir/rk3399_linux_release_v2.5.1_20210301/kernel/
#交叉编译使用对应的Kernel源码目录

CURRENT_PATH := $(shell pwd)
ARCH := arm64

#要生成的模块名
obj-m := adctest.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules ARCH=$(ARCH)
	date
	$(CC) adctestapp.c -o adctestapp -Wall -pthread -O2

install:
	scp adctest adctest.ko root@192.168.192.5:/home/root

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
	rm adctest -rf

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := hps_multi_pwm.o
#CFLAGS_$(obj-m) := -DDEBUG

else
# normal makefile

# default environment variables
KDIR ?= ~/linux-socfpga

# full make command
MAKECMD = $(MAKE) -C $(KDIR) M=$$PWD

default:
	$(MAKECMD)

modules_install:
	$(MAKECMD) modules_install

clean:
	$(MAKECMD) clean
endif

ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := hps_multi_pwm.o
#CFLAGS_$(obj-m) := -DDEBUG

else
# normal makefile

# default environment variables
KDIR ?= ~/linux-socfpga
export ARCH ?= arm
export CROSS_COMPILE ?= arm-linux-gnueabihf-

# full make command
MAKECMD = $(MAKE) -C $(KDIR) M=$$PWD

default:
	$(MAKECMD)

clean:
	$(MAKECMD) clean
endif

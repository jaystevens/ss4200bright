
# To build modules outside of the kernel tree, we run "make"
# in the kernel source tree; the Makefile these then includes this
# Makefile once again.
# This conditional selects whether we are being included from the
# kernel Makefile or not.
ifeq ($(KERNELRELEASE),)

# Assume the source tree is where the running kernel was built
# You should set KERNELDIR in the environment if it's elsewhere
#    KERNELDIR ?= /lib/modules/$(shell uname -r)/build
#    KERNELDIR = /usr/src/kernels/2.6.18-8.el5-i686
    KERNELDIR = /usr/src/kernels/$(shell uname -r)-i686
    # The current directory is passed to sub-makes as argument
    PWD := $(shell pwd)


modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) INC=${CF_PROJECT}/include modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules_install clean

else
    CFLAGS += -I$(INC)
    obj-m := nasgpio.o
endif



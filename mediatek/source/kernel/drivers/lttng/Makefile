CONFIG_LTT_MODULE := m
ifeq ($(TARGET_BUILD_VARIANT),user)
  CONFIG_LTT_MODULE := n
endif
ifeq (eng,$(TARGET_BUILD_VARIANT))
  CONFIG_LTT_MODULE := m
endif

ifneq ($(KERNELRELEASE),)
ifneq ($(CONFIG_MARKERS),)

obj-$(CONFIG_LTT_MODULE) += ltt-core.o
obj-$(CONFIG_LTT_MODULE) += ltt-tracer.o
obj-$(CONFIG_LTT_MODULE) += ltt-marker-control.o

obj-$(CONFIG_LTT_MODULE) += ltt-relay.o
ltt-relay-objs := ltt-relay-lockless.o ltt-relay-alloc.o ltt-relay-splice.o \
		  ltt-relay-vfs.o ltt-event-header.o ltt-ascii.o \
		  ltt-serialize.o ltt-type-serializer.o

obj-$(CONFIG_LTT_MODULE) += ltt-statedump.o
obj-$(CONFIG_LTT_MODULE) += ltt-trace-control.o
obj-$(CONFIG_LTT_MODULE) += ltt-userspace-event.o
obj-$(CONFIG_LTT_MODULE) += ltt-filter.o
obj-$(CONFIG_LTT_MODULE) += ltt-kprobes.o
obj-$(CONFIG_LTT_MODULE) += probes/

endif

else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
	CFLAGS = $(EXTCFLAGS)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install
	/sbin/depmod -a

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

endif


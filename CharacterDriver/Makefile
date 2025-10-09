
obj-m += char_driver.o

# Kernel directory (automatically set by kernel build system)
KDIR := /lib/modules/$(shell uname -r)/build

# Current directory
PWD := $(shell pwd)

# Default target to build the module
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean up generated files
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean


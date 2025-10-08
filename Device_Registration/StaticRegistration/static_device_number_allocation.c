#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kdev_t.h>  // for MAJOR, MINOR, MKDEV
#include <linux/init.h>    // for __init and __exit macros

static int __init static_device_number_allocation(void){
    dev_t device_number;

    // First test with minor = 10
    device_number = MKDEV(0, 10);
    printk(KERN_INFO "Device number: %u\n", (unsigned int)device_number);
    printk(KERN_INFO "Major: %d\n", MAJOR(device_number));
    printk(KERN_INFO "Minor: %d\n", MINOR(device_number));

    // Now create a device number with major = 120, minor = 4
    device_number = MKDEV(120, 4);
    printk(KERN_INFO "Device number: %u\n", (unsigned int)device_number);
    printk(KERN_INFO "Major: %d\n", MAJOR(device_number));
    printk(KERN_INFO "Minor: %d\n", MINOR(device_number));

    return 0;
}

static void __exit static_device_number_cleanup(void){
    printk(KERN_INFO "Exiting static device number module.\n");
}

module_init(static_device_number_allocation);
module_exit(static_device_number_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Statically allocating the device_number\n");
MODULE_AUTHOR("MSS");


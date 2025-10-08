#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>      // for class_create, device_create
#include <linux/fs.h>          // for alloc_chrdev_region

static dev_t device_number;
static struct class* dev_class;

static int __init auto_dev_class_init(void) {
    int ret;

    ret = alloc_chrdev_region(&device_number, 0, 1, "MSS_DEVICE");
    if (ret < 0) {
        printk(KERN_ERR "DEVICE NUMBER REGISTRATION FAILED\n");
        return ret;
    }

    printk(KERN_INFO "DEVICE CREATED, DEVICE_NUMBER: %u , MAJOR : %d, MINOR : %d\n",
           device_number, MAJOR(device_number), MINOR(device_number));

    dev_class = class_create("MSS_CLASS"); // Creating the class in /sys/class
    
    if (IS_ERR(dev_class)) {
        unregister_chrdev_region(device_number, 1);
        printk(KERN_ERR "FAILED TO CREATE CLASS\n");
        return PTR_ERR(dev_class);
    }

    if (device_create(dev_class, NULL, device_number, NULL, "MSS_DEVICE") == NULL) {
        class_destroy(dev_class);
        unregister_chrdev_region(device_number, 1);
        printk(KERN_ERR "FAILED TO CREATE DEVICE\n");
        return -1;
    }

    printk(KERN_INFO "Device class and device created successfully\n");
    return 0;
}

static void __exit cleanup(void) {
    device_destroy(dev_class, device_number); // Destroy device first
    class_destroy(dev_class);
    unregister_chrdev_region(device_number, 1);
    printk(KERN_INFO "EXITING THE AUTOMATIC DEVICE CREATION MODULE\n");
}

module_init(auto_dev_class_init);
module_exit(cleanup);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MSS");
MODULE_DESCRIPTION("AUTOMATIC DEVICE FILE/NODE CREATION");


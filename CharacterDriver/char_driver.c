#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>        // for struct file_operations
#include <linux/kernel.h>    // for printk, pr_info
#include <linux/cdev.h>      // for cdev functions
#include <linux/device.h>    // for class_create, device_create
#include <linux/uaccess.h>   // for copy_to_user, copy_from_user

#define BUFFER_SIZE 1024                 // Size of the kernel buffer
#define DEVICE_NAME "mss_device"         // Device name in /dev
#define CLASS_NAME  "MSS_CLASS"          // sysfs class name

static dev_t device_number;             // Device number (major + minor)
static int count = 1;                   // Number of devices
static int base_minor = 0;              // Starting minor number

static struct class *my_class;         // Device class pointer
static struct cdev my_cdev;            // Character device structure

static char device_buffer[BUFFER_SIZE]; // Device data buffer
static int open_count = 0;              // Counter to track open calls

// --- File operation implementations ---

// my_open function, when the user calls open()
static int my_open(struct inode *inode, struct file *file) {
    open_count++;
    pr_info("mychardev: Device opened %d time(s)\n", open_count);
    return 0;
}

// my_release function, when the user calls close()
static int my_release(struct inode *inode, struct file *file) {
    pr_info("mychardev: Device closed\n");
    return 0;
}

// my_read function, when the reader calls read()
static ssize_t my_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    int bytes_to_read;

    if (*ppos >= BUFFER_SIZE)
        return 0; // End of file

    // Calculating how many bytes to read
    bytes_to_read = BUFFER_SIZE - *ppos;
    if (count < bytes_to_read)
        bytes_to_read = count;

    // Copy data from kernel buffer to user buffer
    if (copy_to_user(buf, device_buffer + *ppos, bytes_to_read))
        return -EFAULT;

    // Print what is being read
    pr_info("mychardev: Read %d bytes: '", bytes_to_read);
    for (int i = 0; i < bytes_to_read; i++) {
        char c = device_buffer[*ppos + i];
        if (c >= 32 && c <= 126) // Printable ASCII
            printk(KERN_CONT "%c", c);
        else
            printk(KERN_CONT "\\x%02x", (unsigned char)c);
    }
    printk(KERN_CONT "'\n");

    *ppos += bytes_to_read; // Update file offset
    return bytes_to_read;
}

// my_write function, when the user calls write()
static ssize_t my_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    int bytes_to_write;

    if (*ppos >= BUFFER_SIZE)
        return -ENOSPC; // No space left

    // Calculate how many bytes to write
    bytes_to_write = BUFFER_SIZE - *ppos;
    if (count < bytes_to_write)
        bytes_to_write = count;

    // Copy data from user buffer to kernel buffer
    if (copy_from_user(device_buffer + *ppos, buf, bytes_to_write))
        return -EFAULT;

    // Print what was written
    pr_info("mychardev: Received %d bytes: '", bytes_to_write);
    for (int i = 0; i < bytes_to_write; i++) {
        char c = device_buffer[*ppos + i];
        if (c >= 32 && c <= 126) // Printable ASCII
            printk(KERN_CONT "%c", c);
        else
            printk(KERN_CONT "\\x%02x", (unsigned char)c);
    }
    printk(KERN_CONT "'\n");

    *ppos += bytes_to_write; // Update file offset
    return bytes_to_write;
}

// Defining file operations structure
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .open = my_open,
    .release = my_release,
};

//--------------init and exit functions------

// char_driver_init - Called when module is loaded
static int __init char_driver_init(void) {
    int ret;

    // dynamically registering the device
    ret = alloc_chrdev_region(&device_number, base_minor, count, DEVICE_NAME);
    if (ret < 0) {
        pr_info("Device registration failed\n");
        return ret;
    }

    pr_info("Device number: %d | MAJOR: %d, MINOR: %d is registered\n",
            device_number, MAJOR(device_number), MINOR(device_number));

    // creating the device_node using automatic way
    my_class = class_create(CLASS_NAME);
    
    pr_info("%s is created in the sysfs\n", CLASS_NAME);

    device_create(my_class, NULL, device_number, NULL, DEVICE_NAME);
    pr_info("Device file created under class: %s\n", CLASS_NAME);

    // initializing the cdev structure
    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    // adding the cdev to the kernel
    ret = cdev_add(&my_cdev, device_number, count);
    if (ret < 0) {
        device_destroy(my_class, device_number);
        class_destroy(my_class);
        unregister_chrdev_region(device_number, count);
        pr_info("Failed to add cdev\n");
        return ret;
    }

    pr_info("cdev_init() and cdev_add() successful\n");
    return 0;
}

// char_driver_exit - Called when module is unloaded
static void __exit char_driver_exit(void) {
    cdev_del(&my_cdev);
    device_destroy(my_class, device_number);
    class_destroy(my_class);
    unregister_chrdev_region(device_number, count);

    pr_info("Character driver module exiting successfully\n");
}

module_init(char_driver_init);
module_exit(char_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("MSS");
MODULE_DESCRIPTION("CREATING A CHARACTER DRIVER");


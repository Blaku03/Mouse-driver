#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "simple_char_dev"
static int device_file_major_number;

static ssize_t device_read(struct file *file, char __user *buffer, size_t length, loff_t *offset) {
    const char *message = "Hello from the kernel!\n";
    size_t message_len = strlen(message);
    printk( KERN_NOTICE "Simple-driver: Device file is read at offset = %d, read bytes count = %u\n", (int)*position, (unsigned int)length );
    if (*offset >= message_len)
        return 0;
    if (length > message_len - *offset)
        length = message_len - *offset;
    if (copy_to_user(buffer, message + *offset, length))
        return -EFAULT;
    *offset += length;
    return length;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = device_read,
};

static int __init simple_char_init(void) {
    device_file_major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (device_file_major_number < 0) {
        printk( KERN_WARNING "Failed to register device\n");
        return device_file_major_number;
    }
    printk( KERN_INFO "Device registered with major number %d\n", device_file_major_number);
    return 0;
}

static void __exit simple_char_exit(void) {
    unregister_chrdev(device_file_major_number, DEVICE_NAME);
    if (device_file_major_number >= 0) {
        printk( KERN_INFO "Device unregistered\n");
    }
}

module_init(simple_char_init);
module_exit(simple_char_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple char device driver");

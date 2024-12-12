#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mouse_position_tracker"
#define CLASS_NAME "mouse_tracker"

static struct input_dev *virtual_mouse_dev;
static int cursor_x = 0, cursor_y = 0;
static int major_number;
static struct class *tracker_class = NULL;
static struct device *tracker_device = NULL;
static char position_buffer[64] = "X=0, Y=0\n";
static int buffer_length;

// Function to update mouse movement and display the position
static void mouse_event_handler(int dx, int dy)
{
    cursor_x += dx;
    cursor_y += dy;

    snprintf(position_buffer, sizeof(position_buffer), "X=%d, Y=%d\n", cursor_x, cursor_y);
    buffer_length = strlen(position_buffer);

    printk(KERN_INFO "Mouse position: %s", position_buffer);
}

// Read function for the character device
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int bytes_to_copy;

    if (*offset >= buffer_length) {
        *offset = 0; // Reset offset at the end of the data
        return 0; // End of data
    }

    bytes_to_copy = min(len, (size_t)(buffer_length - *offset));

    if (copy_to_user(buffer, position_buffer + *offset, bytes_to_copy)) {
        return -EFAULT;
    }

    *offset += bytes_to_copy;
    return bytes_to_copy;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = dev_read,
};

static int __init mouse_tracker_init(void)
{
    int err;

    printk(KERN_INFO "Initializing Mouse Tracker Module\n");

    // Register character device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    tracker_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(tracker_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(tracker_class);
    }

    tracker_device = device_create(tracker_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(tracker_device)) {
        class_destroy(tracker_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(tracker_device);
    }

    // Allocate virtual input device
    virtual_mouse_dev = input_allocate_device();
    if (!virtual_mouse_dev) {
        device_destroy(tracker_class, MKDEV(major_number, 0));
        class_destroy(tracker_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to allocate input device\n");
        return -ENOMEM;
    }

    virtual_mouse_dev->name = DEVICE_NAME;

    // Register input device
    err = input_register_device(virtual_mouse_dev);
    if (err) {
        input_free_device(virtual_mouse_dev);
        device_destroy(tracker_class, MKDEV(major_number, 0));
        class_destroy(tracker_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register input device\n");
        return err;
    }

    // Initialize the buffer length
    buffer_length = strlen(position_buffer);

    printk(KERN_INFO "Mouse Tracker Module initialized successfully\n");

    // Simulate initial movements
    mouse_event_handler(10, 15);
    mouse_event_handler(5, 5);
    // In log after this should be 
    // Mouse position: X=10, Y=15
    // X+5, Y+5
    // Mouse position: X=15, Y=20
    return 0;
}

static void __exit mouse_tracker_exit(void)
{
    printk(KERN_INFO "Exiting Mouse Tracker Module\n");

    input_unregister_device(virtual_mouse_dev);
    device_destroy(tracker_class, MKDEV(major_number, 0));
    class_destroy(tracker_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "Mouse Tracker Module exited\n");
}

module_init(mouse_tracker_init);
module_exit(mouse_tracker_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Mouse Tracker Module that logs cursor position and exposes it via /dev");

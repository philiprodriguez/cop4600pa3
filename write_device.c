#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h> // Has copy to user function
#define DEVICE_NAME "fifodev"
#define CLASS_NAME "fifo"
#define BUFFER_SIZE 1024

static int majorNumber;
struct class * fifoDeviceClass;
struct device * fifoDeviceDevice;

MODULE_LICENSE("GPL");

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static DEFINE_MUTEX(fifo_mutex);

static struct file_operations fops =
{
	.open = dev_open,
	.write = dev_write,
	.release = dev_release
};

// The buffer in which we will store characters!
char queue[BUFFER_SIZE];

// Represents the next open position in our message buffer.
short queueFirstByte;

// Represents the number of bytes currently in the queue.
short queueSize;


int init_module(void)
{
	printk(KERN_INFO "Initializing the FIFO write device...\n");

  mutex_init(&fifo_mutex);

	// Assign a major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

	// Did we succeed?
	if (majorNumber < 0)
	{
		// We failed.
		printk(KERN_ALERT "Failed to assign FIFO write device a major number!\n");
		return majorNumber;
	}
	// We succeeded!
	printk("Registered FIFO write device with major number %d.\n", majorNumber);

	// Register device class
	fifoDeviceClass = class_create(THIS_MODULE, CLASS_NAME);

	// Did we succeed?
	if (IS_ERR(fifoDeviceClass))
	{
		// Nope.
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create FIFO write device class!\n");
		return PTR_ERR(fifoDeviceClass);
	}
	// We succeeded!
	printk(KERN_INFO "Created FIFO write device class.\n");

	// Register the device driver
	fifoDeviceDevice = device_create(fifoDeviceClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

	// Did we succeed?
	if (IS_ERR(fifoDeviceDevice))
	{
		// Nope.
		class_destroy(fifoDeviceClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the FIFO write device!\n");
		return PTR_ERR(fifoDeviceDevice);
	}
	// We succeeded!
	printk(KERN_INFO "Successfully created FIFO write device!\n");

	// Initialize queueSize
	queueSize = 0;

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Cleaning up FIFO write device!\n");
  mutex_destroy(&fifo_mutex);
	device_destroy(fifoDeviceClass, MKDEV(majorNumber, 0));
	class_unregister(fifoDeviceClass);
	class_destroy(fifoDeviceClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);

	printk(KERN_INFO "FIFO device cleaned up!\n");
}

static int dev_open(struct inode * inodep, struct file * filep)
{
  if (!mutex_trylock(&fifo_mutex)) {
    printk(KERN_ALERT "FIFO device is used by another process");
    return -EBUSY;
  }
	printk(KERN_INFO "FIFO device opened.\n");
	return 0;
}

static int dev_release(struct inode * inodep, struct file * filep)
{
  mutex_unlock(&fifo_mutex);
	printk(KERN_INFO "FIFO device closed.\n");
	return 0;
}

static ssize_t dev_write(struct file * filep, const char * buffer, size_t len, loff_t * offset)
{
	int buffer_iterator = 0;
	for (; queueSize < BUFFER_SIZE && buffer_iterator < len; queueSize++)
	{
		// Since size_of_message < BUFFER_SIZE, we have a space available at message[size_of_message] to put a byte!
		queue[(queueFirstByte+queueSize) % BUFFER_SIZE] = buffer[buffer_iterator];
		buffer_iterator++;
	}

	printk(KERN_INFO "%zu bytes sent to FIFO write device, %d bytes written.\n", len, buffer_iterator);
	return len;
}

/*
Philip Rodriguez
Steven Chen
Ryan Beck

Programming Assignment 3
*/

#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#define DEVICE_NAME "fiforeaddev"
#define CLASS_NAME "fiforead"
#define BUFFER_SIZE 1024

static int majorNumber;
struct class * deviceClass;
struct device * deviceDevice;

MODULE_LICENSE("GPL");

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};

extern struct mutex queueMutex;
extern char queue[BUFFER_SIZE];
extern short queueFirstByte;
extern short queueSize;

int init_module(void)
{
	printk(KERN_INFO "Initializing the FIFO read device...\n");

	// Assign a major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);

	// Did we succeed?
	if (majorNumber < 0)
	{
		// We failed.
		printk(KERN_ALERT "Failed to assign FIFO read device a major number!\n");
		return majorNumber;
	}
	// We succeeded!
	printk("Registered FIFO read device with major number %d.\n", majorNumber);

	// Register device class
	deviceClass = class_create(THIS_MODULE, CLASS_NAME);

	// Did we succeed?
	if (IS_ERR(deviceClass))
	{
		// Nope.
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create FIFO read device class!\n");
		return PTR_ERR(deviceClass);
	}
	// We succeeded!
	printk(KERN_INFO "Created FIFO read device class.\n");

	// Register the device driver
	deviceDevice = device_create(deviceClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

	// Did we succeed?
	if (IS_ERR(deviceDevice))
	{
		// Nope.
		class_destroy(deviceClass);
		unregister_chrdev(majorNumber, DEVICE_NAME);
		printk(KERN_ALERT "Failed to create the FIFO read device!\n");
		return PTR_ERR(deviceDevice);
	}
	// We succeeded!
	printk(KERN_INFO "Successfully created FIFO read device!\n");

	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Cleaning up FIFO read device!\n");

	device_destroy(deviceClass, MKDEV(majorNumber, 0));
	class_unregister(deviceClass);
	class_destroy(deviceClass);
	unregister_chrdev(majorNumber, DEVICE_NAME);

	printk(KERN_INFO "FIFO device cleaned up!\n");
}

static int dev_open(struct inode * inodep, struct file * filep)
{
	printk(KERN_INFO "FIFO read device opened.\n");
	return 0;
}

static int dev_release(struct inode * inodep, struct file * filep)
{
	printk(KERN_INFO "FIFO read device closed.\n");
	return 0;
}

static ssize_t dev_read(struct file * filep, char * buffer, size_t len, loff_t * offset)
{
	char * returning;
	int bytesRead;
	int error_count;

	// So first thing's first, we need to "own" the queue!
	mutex_lock(&queueMutex);

	// You want to read more than is in the queue? You don't!
	if (len > queueSize)
	{
		len = queueSize;
	}

	
	returning = kmalloc(len, GFP_KERNEL);
	
	
	for (bytesRead = 0; bytesRead < len; bytesRead++)
	{
		returning[bytesRead] = queue[queueFirstByte];
		queueFirstByte = (queueFirstByte + 1) % BUFFER_SIZE;
		queueSize--;
	}

	error_count = copy_to_user(buffer, returning, len);
	
	// We are done with returning once it is copied into buffer, so free it!
	kfree(returning);

	// Now that we are done with all the queue stuff, we can unlock it.
	mutex_unlock(&queueMutex);

	if (error_count == 0)
	{
		printk(KERN_INFO "%zu bytes read from FIFO read device.\n", len);
	
		// Return the number of bytes removed or read from our queue.
		return len;
	}
	else
	{
		printk(KERN_INFO "Bytes couldn't be read from FIFO read device!\n");
		return -EFAULT;
	}
}

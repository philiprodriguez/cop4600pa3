#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#define  DEVICE_NAME "fiforead"
#define  CLASS_NAME  "fifo_read"
#define BUFFER_SIZE 1024

MODULE_LICENSE("GPL");

static int    majorNumber;
extern char   message[BUFFER_SIZE];

char queue[BUFFER_SIZE];
short queueFirstByte;
short queueSize;


static int majorNumber;
struct class * fifoDeviceClass;
struct device * fifoDeviceDevice;

static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static DEFINE_MUTEX(fifo_mutex);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};


int init_module(void)
{
   printk(KERN_INFO "Initializing the FIFO read device...\n");
    mutex_init(&fifo_mutex);
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
  	fifoDeviceClass = class_create(THIS_MODULE, CLASS_NAME);

  	// Did we succeed?
  	if (IS_ERR(fifoDeviceClass))
  	{
  		// Nope.
  		unregister_chrdev(majorNumber, DEVICE_NAME);
  		printk(KERN_ALERT "Failed to create FIFO read device class!\n");
  		return PTR_ERR(fifoDeviceClass);
  	}
  	// We succeeded!
  	printk(KERN_INFO "Created FIFO read device class.\n");

  	// Register the device driver
  	fifoReadDevice = device_create(fifoDeviceClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);

  	// Did we succeed?
  	if (IS_ERR(fifoReadDevice))
  	{
  		// Nope.
  		class_destroy(fifoDeviceClass);
  		unregister_chrdev(majorNumber, DEVICE_NAME);
  		printk(KERN_ALERT "Failed to create the FIFO read device!\n");
  		return PTR_ERR(fifoReadDevice);
  	}
  	// We succeeded!
  	printk(KERN_INFO "Successfully created FIFO read device!\n");

  	// Initialize queueSize
  	queueSize = 0;

  	return 0;
}

void cleanup_module(void)
{
  mutex_destroy(&fifo_mutex);
	printk(KERN_INFO "Cleaning up FIFO read device!\n");

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
	printk(KERN_INFO "FIFO read device opened.\n");
	return 0;
}

static int dev_release(struct inode * inodep, struct file * filep)
{
  mutex_unlock(&fifo_mutex);
	printk(KERN_INFO "FIFO read device closed.\n");
	return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
  char * returning;
 	int bytesRead;
 	int error_count;

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
 	if (error_count == 0)
 	{
 		printk(KERN_INFO "%zu bytes read from FIFO device.\n", len);
 		return len;
 	}
 	else
 	{
 		printk(KERN_INFO "Bytes couldn't be read from FIFO device!\n");
 		return -EFAULT;
 	}

 	kfree(returning);
}

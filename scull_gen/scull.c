#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/kdev_t.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include "scull.h"

MODULE_AUTHOR("Alessandro Rubini, Jonathan Corbet");
MODULE_LICENSE("Dual BSD/GPL");

// 	module's name.
static const char *name = "scull";
static int scull_major = SCULL_MAJOR;
static int scull_minor = 0;
static dev_t dev;
int scull_quantum = SCULL_QUANTUM;
int scull_qset = SCULL_QSET;
struct scull_dev *scull_devices; 	// allocated in module_init0
static int dev_nr = SCULL_NR_DEVS; 	// number of devices.


/*	Empty out the scull device; must be called
 *	with device semaphore held.	*/

static int scull_trim(struct scull_dev *sc)
{
	struct scull_qset *next, *dptr;
	int qset = sc->qset; // sc is not NULL
	int i;

	for(dptr = sc->data; dptr; dptr = next) { /*all the list items*/
		if(dptr->data) {
			for( i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}

	sc->size = 0;
	sc->quantum = scull_quantum;
	sc->qset = scull_qset;
	sc->data = NULL;

	return (0);
}

static loff_t scull_llseek(struct file *fp, loff_t off, int count)
{
	return off;
}

static ssize_t scull_read(struct file * fp, char __user * ubuf, size_t size, loff_t * offset)
{
	return size;
}

static ssize_t scull_write(struct file *fp, const char __user *ubuf, size_t size, loff_t * offset)
{
	printk(KERN_ALERT "scull_write size(%d)\n",(int) size);
	return size;
}


static int scull_open(struct inode *in, struct file *filp)
{
	struct scull_dev *devv;

	printk(KERN_ALERT "scull_open\n");
	devv = container_of(in->i_cdev, struct scull_dev, cdev);
	filp->private_data = devv;	
	
	/* trim to 0 the length of the device if open as write-only */
	if( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		scull_trim(devv); /*ignore errors */
	}
	
	return (0);
}

static int scull_release(struct inode *in, struct file * filp)
{
	return (0);
}

//	file-operations supported by the char driver.
struct file_operations scull_fops = {
	.owner = 	THIS_MODULE,
	.llseek =	scull_llseek,
	.read = 	scull_read,
	.write = 	scull_write,
	.open = 	scull_open,
	.release = 	scull_release,
};

//	setting-up of the cdev's on the scull_dev's struct members.
static void scull_setup_cdev(struct scull_dev *dev, int index)
{
	int err;
	int devno = MKDEV(scull_major, scull_minor + index);	

	cdev_init(& (dev->cdev), &scull_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &scull_fops;
	err = cdev_add( &(dev->cdev), devno, 1);

	if(err)
		printk(KERN_ALERT "Error %d adding scull%d\n",err, index);
}

// 	cleanup module.
static void module_exit0(void)
{
	unregister_chrdev_region(dev,dev_nr);
	printk(KERN_ALERT "dev: %d nr: %d\n", dev, dev_nr);
	kfree(scull_devices);
	printk(KERN_ALERT " -- Module removed\n");
}

//	init_module.
static int module_init0(void)
{
	int i;	
	int result;	
	
	// we got major, minor and want to get a dev_t back.
	dev = MKDEV(scull_major, 0);

	if( scull_major)
		result = register_chrdev_region(dev, dev_nr, name);
	else {
		result = alloc_chrdev_region(&dev, 0, dev_nr, name); 
		scull_major = MAJOR(dev);
	}
	
	if(result < 0) {
		printk(KERN_ALERT "scull: unable to get major : %d\n", scull_major);
		return result;
	}
	
	/* Allocating memory for the number of the devices */
	scull_devices = kmalloc(dev_nr * sizeof(struct scull_dev), GFP_KERNEL);

	if(!scull_devices) {
		printk(KERN_ALERT "kmalloc failed\n");
		result = - ENOMEM;
		goto fail;
	}
	
	// `mem-settin'` to 'zeros'.	
	memset(scull_devices, 0, dev_nr * sizeof(struct scull_dev));

	if(scull_major == 0)
		scull_major = result;

	/* Initializing each device */
	for(i = 0; i < dev_nr; i++) {
		scull_setup_cdev(&scull_devices[i], i);
	}
	
	printk(KERN_ALERT "--Module loaded into kernel.\n");
	printk(KERN_ALERT "dev: %d\n", dev);	
	return (0);
fail:
	module_exit0();
	return result;
}
module_init(module_init0);
module_exit(module_exit0);


#ifndef _SCULL_H_
#define _SCULL_H_

#include <linux/ioctl.h> // needed for _IOW etc stuff used latter on.

#ifndef SCULL_MAJOR
#define SCULL_MAJOR 0
#endif

//	dynamic major by default.
#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4
#endif

//	scull0-scull3
#ifndef SCULL_NR_DEVS
#define SCULL_NR_DEVS 4
#endif


/*	The bare device is a variable lenght region of memory.
 *	Use a linked list of indirect blocks.
 *
 *	'scull_dev->data'  points to an array of pointers, each
 *	pointer refers to a memory area of SCULL_QUANTUM bytes.
 *	
 *	The array(quantum-set) is SCULL_QSET long.
 *
 */

#ifndef SCULL_QUANTUM
#define SCULL_QUANTUM 4000
#endif

#ifndef SCULL_QSET
#define SCULL_QSET 1000
#endif

//	Representation of scull quantum sets.
struct scull_qset {
	void **data;
	struct scull_qset *next;
};

//	scull device's	struct.
struct scull_dev {
	struct scull_qset *data; 	/* Pointer to first quantum set	*/
	int quantum;			/* the current quantum size	*/
	int qset;		 	/* the current array size	*/
	unsigned long size;		/* amount of data stored here 	*/
	unsigned int access_key;	/* used by scullid and scullpriv*/
	struct semaphore sem;		/* mutual exclusion semaphore 	*/
	struct cdev cdev;		/* Char device structure	*/
};

#define TYPE(minor)	( ( (minor) >> 4) & 0xf)  // high nibble.
#define NUM(minor)	( (minor) & 0xf)	  //low nibble.

#endif /* _SCULL_H_ */

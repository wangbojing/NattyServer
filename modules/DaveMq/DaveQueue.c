/*
 *  Author : WangBoJing , email : 1989wangbojing@gmail.com
 * 
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Author. (C) 2016
 * 
 *
 
****       *****
  ***        *
  ***        *                         *               *
  * **       *                         *               *
  * **       *                         *               *
  *  **      *                        **              **
  *  **      *                       ***             ***
  *   **     *       ******       ***********     ***********    *****    *****
  *   **     *     **     **          **              **           **      **
  *    **    *    **       **         **              **           **      *
  *    **    *    **       **         **              **            *      *
  *     **   *    **       **         **              **            **     *
  *     **   *            ***         **              **             *    *
  *      **  *       ***** **         **              **             **   *
  *      **  *     ***     **         **              **             **   *
  *       ** *    **       **         **              **              *  *
  *       ** *   **        **         **              **              ** *
  *        ***   **        **         **              **               * *
  *        ***   **        **         **     *        **     *         **
  *         **   **        **  *      **     *        **     *         **
  *         **    **     ****  *       **   *          **   *          *
*****        *     ******   ***         ****            ****           *
                                                                       *
                                                                      *
                                                                  *****
                                                                  ****


 *
 */



#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/io.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include <linux/spinlock.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "DaveQueueDev.h"

static int DaveMq_major = DAVE_DEV_MAJOR;
module_param(DaveMq_major, int, S_IRUGO);

struct DaveMq_dev *pDaveMqDev;
struct cdev cdev;
//static int DaveMessageQueueCount = 0;
static atomic_t DaveMessageQueueCount = ATOMIC_INIT(0);
spinlock_t DaveMqCountLock = SPIN_LOCK_UNLOCKED;

static DECLARE_WAIT_QUEUE_HEAD(DaveMessageQueue_waitq);


int DaveMq_open(struct inode *inode, struct file *filp) {
	struct DaveMq_dev *dev;

	int minor = MINOR(inode->i_rdev);
	if (minor >= DAVE_DEV_COUNTS) {
		return -ENODEV;
	}
	dev = &pDaveMqDev[minor];
	filp->private_data = dev;

	return 0;
}

int DaveMq_release(struct inode *inode, struct file *filp) {
	return 0;
}


static ssize_t DaveMq_read(struct file *filp, char __user *buf, size_t size, loff_t *ppos) {
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct DaveMq_dev *dev = filp->private_data;

	if (p >= DAVEMQDEV_MEMORY_SIZE) {
		return 0;
	}
	if (count > DAVEMQDEV_MEMORY_SIZE - p) {
		count = DAVEMQDEV_MEMORY_SIZE - p;
	}

	if (copy_to_user(buf, (void*)(dev->data+p), count)) {
		ret = -EFAULT;
	} else {
		*ppos += count;
		ret = count;
		printk(KERN_INFO "read %d byte(s) from %ld\n", count, p);
	}

	spin_lock(&DaveMqCountLock);
	if (atomic_dec_and_test(&DaveMessageQueueCount)) {
		//atomic_inc(&DaveMessageQueueCount);
		ret = -EAGAIN;
	} else {
		atomic_dec(&DaveMessageQueueCount);
	}
	spin_unlock(&DaveMqCountLock);
	//DaveMessageQueueCount = DaveMessageQueueCount > 0 ? DaveMessageQueueCount-1 : 0;
	return ret;
}

static ssize_t DaveMq_write(struct file *filp, const char __user *buf, size_t size, loff_t *ppos) {
	unsigned long p = *ppos;
	unsigned int count = size;
	int ret = 0;
	struct DaveMq_dev *dev = filp->private_data;

	if (p >= DAVEMQDEV_MEMORY_SIZE) {
		return 0;
	}
	if (count > DAVEMQDEV_MEMORY_SIZE - p) {
		count = DAVEMQDEV_MEMORY_SIZE - p;
	}
	if (copy_from_user(dev->data + p, buf, count)) {
		ret = -EFAULT;
	} else {
		*ppos += count;
		ret = count;
		printk(KERN_INFO "written %d byte(s) from %ld\n", count, p);
	}
	spin_lock(&DaveMqCountLock);
	atomic_inc(&DaveMessageQueueCount);
	spin_unlock(&DaveMqCountLock);
	//DaveMessageQueueCount ++;
	return ret;
}

static loff_t DaveMq_llseek(struct file *filp, loff_t offset, int whence) {
	loff_t newpos;

	switch (whence) {
		case 0: { /* SEEK_SET */
			newpos = offset;
			break;
		}
		case 1: { /* SEEK_CUR */
			newpos = filp->f_pos + offset;
			break;
		}
		case 2: { /* SEEK_END */
			newpos = DAVEMQDEV_MEMORY_SIZE - 1 + offset;
			break;
		}
		default:
			return -EINVAL;
	}
	if ((newpos < 0) || (newpos > DAVEMQDEV_MEMORY_SIZE)) {
		return -EINVAL;
	}
	filp->f_pos = newpos;

	return newpos;
}

static unsigned int DaveMq_poll(struct file *file, poll_table *wait) {
	unsigned int mask = 0;
	int Count = 0;
	poll_wait(file, &DaveMessageQueue_waitq, wait);

	spin_lock(&DaveMqCountLock);
	Count = atomic_read(&DaveMessageQueueCount);
	spin_unlock(&DaveMqCountLock);
	
	if (Count > 0) {
		mask |= (POLLIN | POLLRDNORM);
	} else {
		mask = 0;
	}
	return mask;
}


static int DaveMq_mmap(struct file *filp, struct vm_area_struct *vma) {
	struct DaveMq_dev *dev = filp->private_data;
	vma->vm_flags |= VM_IO;
	vma->vm_flags |= (VM_DONTEXPAND | VM_DONTDUMP); //kernel 3.7 don't support VM_RESERVED

	if (remap_pfn_range(vma, vma->vm_start, virt_to_phys(dev->data) >> PAGE_SHIFT, vma->vm_end-vma->vm_start, vma->vm_page_prot)) {
		return -EAGAIN;
	}
	return 0;
}

static const struct file_operations DaveMq_fops = {
	.owner = THIS_MODULE,
	.llseek = DaveMq_llseek,
	.read = DaveMq_read,
	.write = DaveMq_write,
	.open = DaveMq_open,
	.release = DaveMq_release,
	.poll = DaveMq_poll,
	.mmap = DaveMq_mmap,
};

static int __init DaveMq_init(void) {
	int result;
	int i;

	dev_t devno = MKDEV(DaveMq_major, 0);
	if (DaveMq_major) {
		result = register_chrdev_region(devno, 2, "DaveMqDev");
	} else {
		result = alloc_chrdev_region(&devno, 0, 2, "DaveMqDev");
		DaveMq_major = MAJOR(devno);
	}

	if (result < 0) {
		return result;
	}

	cdev_init(&cdev, &DaveMq_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &DaveMq_fops;
	cdev_add(&cdev, MKDEV(DaveMq_major, 0), DAVE_DEV_COUNTS);

	pDaveMqDev = kmalloc(DAVE_DEV_COUNTS * sizeof(struct DaveMq_dev), GFP_KERNEL);
	if (!pDaveMqDev) {
		result = -ENOMEM;
		goto fail_malloc;
	}
	memset(pDaveMqDev, 0, sizeof(struct DaveMq_dev));
	for (i = 0;i < DAVE_DEV_COUNTS;i ++) {
		pDaveMqDev[i].size = DAVEMQDEV_MEMORY_SIZE;
		pDaveMqDev[i].data = kmalloc(DAVEMQDEV_MEMORY_SIZE, GFP_KERNEL);
		memset(pDaveMqDev[i].data, 0, DAVEMQDEV_MEMORY_SIZE);
	}

	return 0;
	
fail_malloc:
	unregister_chrdev_region(devno, 1);
	printk("init module \n");
	return result;
}

static void __exit DaveMq_exit(void) {
	cdev_del(&cdev);
	kfree(pDaveMqDev);
	unregister_chrdev_region(MKDEV(DaveMq_major, 0), 2);
	printk("exit module \n");
}

MODULE_AUTHOR("WangBoJing");
MODULE_LICENSE("GPL");
module_init(DaveMq_init);
module_exit(DaveMq_exit);





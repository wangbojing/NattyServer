

#ifndef __DAVEQUEUE_DEVICE_H__
#define __DAVEQUEUE_DEVICE_H__

#ifndef DAVE_DEV_MAJOR
#define DAVE_DEV_MAJOR		247
#endif

#ifndef DAVE_DEV_COUNTS
#define DAVE_DEV_COUNTS		2
#endif

#ifndef DAVEMQDEV_MEMORY_SIZE
#define DAVEMQDEV_MEMORY_SIZE	4096
#endif

struct DaveMq_dev {
	char *data;
	unsigned long size;
};


#endif




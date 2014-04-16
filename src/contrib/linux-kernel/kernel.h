/*
  File taken from the Linux kernel v3.14
  Slightly modified in order to work in userspace and without the rest of
  the kernel sources. Stripped down to the minimum.
*/

#ifndef _LINUX_KERNEL_H
#define _LINUX_KERNEL_H

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:	the pointer to the member.
 * @type:	the type of the container struct this is embedded in.
 * @member:	the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})

#endif

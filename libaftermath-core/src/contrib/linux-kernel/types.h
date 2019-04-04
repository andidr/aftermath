/*
  File taken from the Linux kernel v3.14
  Slightly modified in order to work in userspace and without the rest of
  the kernel sources.
*/

#ifndef _LINUX_TYPES_H
#define _LINUX_TYPES_H

#define __EXPORTED_HEADERS__

struct list_head {
	struct list_head *next, *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#endif /* _LINUX_TYPES_H */

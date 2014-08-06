Xinu-Demand-Paging
==================

Virtual Memory implementation using XINU. Edits made in the 'Paging' folder to implement following system calls among others:
==SYSCALL xmmap (int virtpage, bsd_t source, int npages)
 map a source file of size npages pages to the virtual page virtpage.
==SYSCALL xmunmap (int virtpage)
 remove a virtual memory mapping.
==SYSCALL vcreate (int *procaddr, int ssize, int hsize, int priority, char *name, int nargs, long args)
 create a new Xinu process.
==WORD *vgetmem (int nbytes)
vgetmem() will allocate the desired amount of memory if possible;
get the memory from a process' private heap located in virtual memory.

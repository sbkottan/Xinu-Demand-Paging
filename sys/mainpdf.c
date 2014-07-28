/* user.c - main */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <stdio.h>
#include <paging.h>
void halt();
/*------------------------------------------------------------------------
* main -- user main program
*------------------------------------------------------------------------
*/
int main()
{
char *addr = (char*)0x40000000; //1G
bsd_t bs=1;
int i= ((unsigned long)addr)>>12; // the ith page
kprintf("\n\nHello World, Xinu lives\n\n");
kprintf("Calling get_bs for bs: %d npages:200\n",bs);
get_bs(bs, 200);
kprintf("Calling xmmap for vpage:%x(%d) bs:%d npages:200\n",i,i,bs);
if (xmmap(i, bs, 200) == SYSERR) {
kprintf("xmmap call failed\n");
return 0;
}
for(i=0;i<2;i++){
kprintf("Write Operation\n==================\n");
*addr = 'A'+i;
addr+=NBPG;
}
kprintf("\nRead Operation\n==================\n");
addr = (char*)0x40000000; //1G
kprintf("0x%08x: %c\n", addr, *addr);
xmunmap(0x40000000>>12);
return 0;
}

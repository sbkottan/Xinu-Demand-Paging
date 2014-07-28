/* bsm.c - manage the backing store mapping*/

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

bs_map_t bs_map[NBS];
bs_t bs_tab[NBS];

/*-------------------------------------------------------------------------
 * init_bsm- initialize bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_bsm()
{
	int i = 0, j = 0;
	for(i = 0;i < NBS; i++)
	{
		bs_tab[i].status = BSM_UNMAPPED;
		bs_tab[i].as_heap = 0;
		bs_tab[i].npages = -1;
		bs_tab[i].owners = NULL;
		bs_tab[i].frm = NULL;
		for(j = 0 ; j < NUMBER_BSTORE_PAGES; j++)
			bs_tab[i].pg_to_frm_map[j] = -1; /* init with no mapping  */
	}
	return OK;
}

/*-------------------------------------------------------------------------
 * get_bsm - get a free entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL get_bsm(int* avail) {
	int i;
	for (i = 0; i < NBS; i++) {
		if (bs_tab[i].status == BSM_UNMAPPED) {
			return *avail = i;
		return OK;
		}
	}
	return SYSERR;
}



/*-------------------------------------------------------------------------
 * free_bsm - free an entry from bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL free_bsm(int i)
{
	bs_tab[i].status = BSM_UNMAPPED;
	bs_tab[i].npages = 0;
	bs_tab[i].as_heap = 0;
	return OK;
}

/*-------------------------------------------------------------------------
 * bsm_lookup - lookup bsm_tab and find the corresponding entry
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_lookup(int pid, long vaddr, int* store, int* pageth)
{
	int  i;
	struct pentry *pptr = &proctab[pid];
	for(i = 0 ; i< NBS; i++){
		bs_map_t *map = &(pptr->map[i]);
		if(map->status == BSM_MAPPED && (((map->vpno + map->npages) * NBPG) >= vaddr) &&
				(map->vpno  * NBPG <= vaddr)){
			*store = i;
			*pageth = find_page(map->vpno, map->npages, vaddr);
			return OK;
		}
	}
	return SYSERR;
}


/*-------------------------------------------------------------------------
 * bsm_map - add an mapping into bsm_tab 
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_map(int pid, int vpno, int source, int npages) {

	if (bs_tab[source].status == BSM_UNMAPPED) {
		bs_tab[source].status = BSM_MAPPED;
		bs_tab[source].npages = npages;
	}

	struct pentry *pptr = &proctab[pid];
	bs_map_t *map = &(pptr->map[source]);
	map->bs = source;
	map->pid = pid;
	map->npages = npages;
	map->vpno = vpno;
	map->status = BSM_MAPPED;

	bs_t *tab = &bs_tab[source];
	bs_map_t *owner = (bs_map_t *)getmem(sizeof(bs_map_t));
	owner->bs = source;
	owner->npages = npages;
	owner->pid = pid;
	owner->vpno = vpno;
	owner->next = tab->owners;
	tab->owners = owner;
	return OK;
}



/*-------------------------------------------------------------------------
 * bsm_unmap - delete an mapping from bsm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL bsm_unmap(int pid, int vpno, int flag)
{
	int i, store, pageth;
	struct pentry *pptr = &proctab[pid];
	bsm_lookup(pid, vpno*NBPG, &store, &pageth);
	bs_map_t *map = &(pptr->map[store]);
	frame_t *frms= map->frm;
	while(frms != NULL){
		frame_t *temp = frms;
		frms = frms->bs_next;
		free_frm(temp);
	}
//	remove_owner_mapping(map->bs, pid);


bs_t *tab = &bs_tab[map->bs];
  bs_map_t *prev = NULL;
  bs_map_t *temp = tab->owners;
  while(temp != NULL){
    if(temp->pid == pid){
      if(temp == tab->owners){
        tab->owners = NULL;
      }
      else{
        prev->next = temp->next;
        temp->next = NULL;
      }
      freemem((struct mblock *)temp, sizeof(bs_map_t));
      return OK;
    }
    prev = temp;
    temp = temp->next;
  }
  return SYSERR;





	map->frm =  NULL;
	map->bs = -1;
	map->npages = 0;
	map->status = BSM_UNMAPPED;
	map->vpno = 0;
	if(bs_tab[store].owners == NULL)
		free_bsm(store);
return OK;
}

int find_page(int start_vpage, int npages, int vaddr){
	int  i;
	for(i = start_vpage; i < (start_vpage + npages); i++){
		if(vaddr >= i*NBPG && vaddr <= (i*NBPG + (NBPG - 1))){
			 i -= start_vpage;
			 return i;
		}
	}
	return (int)0;
}

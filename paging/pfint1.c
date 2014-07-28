/* pfint.c - pfint */

#include <conf.h>
#include <kernel.h>
#include <paging.h>
#include <proc.h>

/*-------------------------------------------------------------------------
 * pfint - paging fault ISR
 *-------------------------------------------------------------------------
 */
SYSCALL pfint() {

	STATWORD ps;
	disable(ps);
	int store, pageth;
	unsigned long vaddr = read_cr2();
	bsm_lookup(currpid, vaddr, &store, &pageth);

frame_t *bs_frm = obtain_frame();
//bsd_t store;
bs_tab[store].pg_to_frm_map[pageth] = bs_frm->frm_num;
bs_frm->bs = store;
		bs_frm->bs_page = pageth;
		bs_frm->status = FRM_BS;
		bs_frm->fr_type = FR_PAGE;
		read_bs((char *)(bs_frm->frm_num * 4096), store, pageth );

	frame_t * frm = bs_frm;








	unsigned int pg_dir_offset = (vaddr & 0xFFC00000) >> 22;
	unsigned int pg_tbl_offset = (vaddr & 0x3FF000) >> 12;
//	unsigned long pdbr = add_pg_dir_entry_for_pg_fault(currpid, pg_dir_offset, pg_tbl_offset, frm);

unsigned long pg_tbl_frm;
	int avail = 0;
	struct pentry *pptr = &proctab[currpid];
	frame_t *pg_dir = pptr->pd;
	pd_t *tmp1 = (pd_t *) ((NBPG * pg_dir->frm_num) + pg_dir_offset * sizeof(pd_t));
	if (tmp1->pd_pres == 1) {
		frame_t *pg_tbl = get_frame(tmp1->pd_base);
		add_pagetable(pg_tbl, pg_tbl_offset, frm->frm_num);
	}
	else{
		// create pd entry as it is absent
		frame_t * pg_tbl = new_table(currpid);
		add_into_directory(pg_dir, pg_dir_offset, pg_tbl->frm_num);
		add_pagetable(pg_tbl, pg_tbl_offset, frm->frm_num);
		pg_tbl_frm = pg_tbl->frm_num ;
	}
    unsigned long pdbr= pptr->pdbr;





























//	add_mapping_to_proc_frm_list(frm, store, currpid);

//frame_t *frm;

struct pentry *pptr1 = &proctab[currpid];
	bs_map_t *map = &(pptr1->map[store]);
	if(map->frm == NULL)
		map->frm = frm;
	else{
		frame_t *tmp = map->frm;
		while(tmp->bs_next != NULL)
			tmp = tmp->bs_next;
		tmp->bs_next = frm;
	}
	frm->fr_vpno = map->vpno + frm->bs_page;
	frm->fr_pid = currpid;








	write_cr3(pdbr * NBPG);
	restore(ps);
	kprintf("faulting on addr %x for proc %d and cr3 = %d\ and page  = %d and vpno = %d\n",
			vaddr, currpid, read_cr3(), pageth, frm->fr_vpno );
//	kprintf("assigned frm num %d for addr %x\n", frm->frm_num, vaddr);
	return OK;
}

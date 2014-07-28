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
	unsigned int p_offset = (vaddr & 0xFFC00000) >> 22;
	unsigned int q_offset = (vaddr & 0x3FF000) >> 12;
	
	bsm_lookup(currpid, vaddr, &store, &pageth);

	frame_t *frame_backing_store = obtain_frame();

	bs_tab[store].pg_to_frm_map[pageth] = frame_backing_store->frm_num;
	frame_backing_store->bs = store;
	frame_backing_store->bs_page = pageth;
	frame_backing_store->status = FRM_BS;
	frame_backing_store->fr_type = FR_PAGE;
	read_bs((char *)(frame_backing_store->frm_num * 4096), store, pageth );
	frame_t * obtained_free_frame = frame_backing_store;

	
//	unsigned long pdbr = add_pg_dir_entry_for_pg_fault(currpid, p_offset, q_offset, obtained_free_frame);

	unsigned long page_to_frame;
	int avail = 0;
	struct pentry *pptr = &proctab[currpid];
	frame_t *directory = pptr->pd;
	pd_t *table_descriptor = (pd_t *) ((4096 * directory->frm_num) + p_offset * sizeof(pd_t));
	if (table_descriptor->pd_pres == 1) 
	{

	frame_t *page_frame;
	int i = 0;
	for (i = 0; i < NFRAMES; i++) {
		if (frm_tab[i].frm_num == table_descriptor->pd_base)
		{
		 page_frame= &frm_tab[i];
		}
	}	

	pt_t *offset_value = (pt_t *) (4096 * page_frame->frm_num );
	offset_value =offset_value+q_offset;
	pt_t ptr;
	ptr.pt_pres = 1;
	ptr.pt_write = 1;
	ptr.pt_user = 0;
	ptr.pt_pwt = 0;
	ptr.pt_pcd = 0;
	ptr.pt_acc = 0;
	ptr.pt_dirty = 0;
	ptr.pt_mbz = 0;
	ptr.pt_global = 0;
	ptr.pt_avail = 0;
	ptr.pt_base = obtained_free_frame->frm_num;
	*offset_value = ptr;


	}
	else
	{
	
	frame_t *page_frame = obtain_frame();
	page_frame->status = FRM_PGT;
	page_frame->fr_type = FRM_PGT;
	page_frame->fr_pid = currpid;

	pd_t *table_descriptor = (pd_t *) (4096 * directory->frm_num);
	table_descriptor =table_descriptor+p_offset;
	pd_t ptr1;
	ptr1.pd_pres = 1;
	ptr1.pd_write = 1;
	ptr1.pd_user = 0;
	ptr1.pd_pwt = 0;
	ptr1.pd_pcd = 0;
	ptr1.pd_acc = 0;
	ptr1.pd_mbz = 0;
	ptr1.pd_fmb = 0;
	ptr1.pd_global = 0;
	ptr1.pd_avail = 0;
	ptr1.pd_base = page_frame->frm_num;
	*table_descriptor = ptr1;

	pt_t *offset_value = (pt_t *) (4096 * page_frame->frm_num );
	offset_value =offset_value+q_offset;
	pt_t ptr;
	ptr.pt_pres = 1;
	ptr.pt_write = 1;
	ptr.pt_user = 0;
	ptr.pt_pwt = 0;
	ptr.pt_pcd = 0;
	ptr.pt_acc = 0;
	ptr.pt_dirty = 0;
	ptr.pt_mbz = 0;
	ptr.pt_global = 0;
	ptr.pt_avail = 0;
	ptr.pt_base = obtained_free_frame->frm_num;
	*offset_value = ptr;
	page_to_frame = page_frame->frm_num ;
	}

	unsigned long pdbr= pptr->pdbr;


	struct pentry *pptr1 = &proctab[currpid];
	bs_map_t *map = &(pptr1->map[store]);
	if(map->frm == NULL)
		map->frm = obtained_free_frame;
	else{
		frame_t *tmp = map->frm;
		while(tmp->bs_next != NULL)
			tmp = tmp->bs_next;
		tmp->bs_next = obtained_free_frame;
	}
	obtained_free_frame->fr_vpno = map->vpno + obtained_free_frame->bs_page;
	obtained_free_frame->fr_pid = currpid;

	write_cr3(pdbr * NBPG);
	restore(ps);
	return OK;
}


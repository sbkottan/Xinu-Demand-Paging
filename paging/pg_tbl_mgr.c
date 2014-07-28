#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>




frame_t * new_table(int pid){
frame_t *pg_tbl = obtain_frame();
	pg_tbl->status = FRM_PGT;
	pg_tbl->fr_type = FRM_PGT;
	pg_tbl->fr_pid = pid;
	return pg_tbl;
}


void add_pagetable(frame_t *frm, int idx, int base){
	pt_t *tmp = (pt_t *) (NBPG * frm->frm_num );
	tmp += idx;
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
	ptr.pt_base = base;
	*tmp = ptr;
}

void remove_pagetable(frame_t *frm, int idx){
	pt_t *tmp = (pt_t *) (NBPG * frm->frm_num);
	tmp += idx;
	pt_t ptr;
	ptr.pt_pres = 0;
	ptr.pt_write = 0;
	ptr.pt_user = 0;
	ptr.pt_pwt = 0;
	ptr.pt_pcd = 0;
	ptr.pt_acc = 0;
	ptr.pt_dirty = 0;
	ptr.pt_mbz = 0;
	ptr.pt_global = 0;
	ptr.pt_avail = 0;
	ptr.pt_base = 0;
	*tmp = ptr;
}

void add_into_directory(frame_t *frm, int idx, int base){
	pd_t *tmp1 = (pd_t *) (NBPG * frm->frm_num);
	tmp1 += idx;
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
	ptr1.pd_base = base;
	*tmp1 = ptr1;
}


SYSCALL create_directory(frame_t *frm, int pid) {
	int avail = 0, i;
	frm->fr_pid = pid;
	frm->fr_type = FR_DIR;
	frm->status = FRM_PGD;

	for (i = 0; i < NUM_GLB_PG_TBLS; i++) {
		add_into_directory(frm, i, glb_pg_tbl_frm_mapping[i]);
	}
	return OK;
}

SYSCALL free_pg_dir(frame_t *pd){
	int i;
	pd_t *ptr1 = (pd_t *)(NBPG * pd->frm_num);
	// dont free global pages
	ptr1 += NUM_GLB_PG_TBLS;
	// free the page tables
	for(i = 0; i < PAGE_TABLES-NUM_GLB_PG_TBLS; i++){
		if(ptr1->pd_pres == 1 ){
			free_frm(get_frame(ptr1->pd_base));
		}
		ptr1++;
	}
	// free the dir
	free_frm(get_frame(pd->frm_num));
	pd = NULL;
	return OK;
}



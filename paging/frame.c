/* frame.c - manage physical frames */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <paging.h>

#define NUM_PAGES_TEST 20

//fr_map_t frm_map[NFRAMES];
frame_t frm_tab[NFRAMES];
frame_t *available_frames;
occupied_frm_list returned_frames;

/*-------------------------------------------------------------------------
 * init_frm - initialize frm_tab
 *-------------------------------------------------------------------------
 */
SYSCALL init_frm()
{
  int i = 0;
  for(i = 0; i < NFRAMES; i++)
  {
                frm_tab[i].status = FRM_FREE;
                                frm_tab[i].bs = -1;
                frm_tab[i].bs_page = -1;
                frm_tab[i].bs_next = NULL;
                frm_tab[i].fifo = NULL;
                                frm_tab[i].refcnt = 0;
                frm_tab[i].age = 0;
                frm_tab[i].frm_num = FRAME0 + i;

                        frame_t *frame=&frm_tab[i];
                        frame->fifo = NULL;
                if(available_frames == NULL)
                available_frames = frame;
                 else{
                frame_t *creat_frame = available_frames;
                while(creat_frame->fifo != NULL)
                        creat_frame= creat_frame->fifo;
                creat_frame->fifo = frame;
                        }

  }
  returned_frames.head = returned_frames.tail = NULL;
  return OK;
}

/*-------------------------------------------------------------------------
 * get_frm - get a free frame according page replacement policy
 *-------------------------------------------------------------------------
 */
SYSCALL get_frm(int* avail)
{
        int i = 0;
        for (i = 0; i < NFRAMES; i++) {
                if(frm_tab[i].status == FRM_FREE){
                        *avail = i;
                        return OK;
                }
        }
  return SYSERR;
}

/*-------------------------------------------------------------------------
 * free_frm - free a frame
 *-------------------------------------------------------------------------
 */
SYSCALL free_frm(frame_t *frm)
{
        if(frm->fr_type == FR_PAGE){
                bs_tab[frm->bs].pg_to_frm_map[frm->bs_page] = -1;
                
                write_bs((char *)(frm->frm_num * 4096), frm->bs, frm->bs_page);
               


 				int i;
                for(i = frm->fr_vpno; i < frm->fr_vpno + 1; i++){
                        unsigned long vaddr = i * NBPG;
                unsigned int pd_offset = (vaddr & 0xFFC00000) >> 22;
                unsigned int pt_offset = (vaddr & 0x3FF000) >> 12;

                 pd_t *tmp1 = (pd_t *) ((NBPG * proctab[frm->fr_pid].pd->frm_num) + pd_offset * sizeof(pd_t));
                 if(tmp1->pd_pres == 1){
                        frame_t * pg_tbl = get_frame(tmp1->pd_base);
                        remove_pagetable(pg_tbl, pt_offset);

                }
               }
                 write_cr3(proctab[currpid].pdbr * NBPG);



        struct pentry *pptr = &proctab[frm->fr_pid];
        bs_map_t *map = &(pptr->map[frm->bs]);
        frame_t *curmap= map->frm;
        frame_t *prevmap= NULL;
        if(curmap == NULL)
        {
        return;
        }

        while(curmap != NULL){
        if(curmap == frm){
                if(curmap == map->frm)
                                map->frm = curmap->bs_next;
                prevmap->bs_next = curmap->bs_next;
                curmap->bs_next = NULL;
                break;
                }
                prevmap = curmap;
                curmap = curmap->bs_next;
        }

    }


frame_t *prevmap = returned_frames.head;
        frame_t *curmap = returned_frames.head;
        while(curmap != NULL ){
                if(curmap == frm){
                        prevmap->fifo = curmap->fifo;
                        if(curmap == returned_frames.head){
                                returned_frames.head = curmap->fifo;
                        }
                        if(curmap == returned_frames.tail){
                                returned_frames.tail = prevmap;
                        }
                        curmap->fifo = NULL;
                        return;
                }
                prevmap= curmap;
                curmap = curmap->fifo;
        }


frm->fifo = NULL;
        if(available_frames == NULL)
                available_frames = frm;
        else{
                frame_t *frame = available_frames;
                while(frame->fifo != NULL)
                        frame = frame->fifo;
                frame->fifo = frm;
        }

        frm->bs = -1;
        frm->bs_page = -1;
        frm->status = FRM_FREE;
        return OK;
}



frame_t *obtain_frame(){
        frame_t * frm = frame_available_frames();
        if(frm == NULL){
                frm = get_evicted_pg();
        }
        return frm;

}




frame_t * get_evicted_pg(){
    frame_t *frm;
    if (grpolicy() == FIFO)
        frm = fifo_evict_policy();
    else{
        frm = fifo_evict_policy();
    }

    free_frm(frm); 

if (available_frames == NULL) {
                return;
        }
        frame_t *curmap = available_frames;
        frame_t *prevmap = available_frames;
        while(curmap != NULL){
                if(curmap == frm){
                        if(available_frames == curmap)
                                available_frames = curmap->fifo;
                        prevmap->fifo = curmap->fifo;
                        curmap->fifo = NULL;
                        return;
                }
                prevmap = curmap;
                curmap = curmap->fifo;
        }

frm->fifo = NULL;
        if(returned_frames.head == NULL){
                returned_frames.head = returned_frames.tail = frm;
        }
        returned_frames.tail->fifo = frm;
        returned_frames.tail = frm;



//    debug_print("replacing frame = %d\n", frm->frm_num);
    return frm;
}

frame_t *fifo_evict_policy(){
    frame_t *curmap = returned_frames.head;
    while(curmap != NULL){
        if(curmap->fr_type == FR_PAGE) return curmap;
        curmap = curmap->fifo;
    }
    return 0;
}





frame_t *frame_available_frames(){
        if(available_frames == NULL){
                return NULL;
        }
        
        frame_t * frame = available_frames;
        available_frames = frame->fifo;
		frame->fifo = NULL;
		if(returned_frames.head == NULL){
                returned_frames.head = returned_frames.tail = frame;
        }
        returned_frames.tail->fifo = frame;
        returned_frames.tail = frame;


        return frame;
}






frame_t *get_frame(int frm_num){
        int i = 0;
        for (i = 0; i < NFRAMES; i++) {
                if (frm_tab[i].frm_num == frm_num) {
                        return &frm_tab[i];
                }
        }
        return 0;
}






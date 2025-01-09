/*
page fault handler 수정해야함
>>
fault exception을 야기한 page에 대해 frame을 할당 받고 system이 해당
instruction을 다시 실행시켜 hit 시키기.
*/

/*
un-mapped가 아닌 Ungrown Stack 부분에서 fault 발생 시 
page 단위로 Stack Growth 수행 이것도 핸들러에서?
*/

/*
1. Supplmental Page Table 구현 --> pagedir.c

VPN, PFN 정보
load 여부
연결된 file, 그 파일이 어디까지 읽었는지 정보
해당 page가 page replacement의 대상이 될 경우 필요한 자료구조 추가
===============================================================
PTE를 관리하기 위한 자료구조는 List로
Process마다 할당되고 해제
Program Binary File을 Page단위로 쪼개고 각각 PTE 생성

2. page fault handler 수정

Memory Access 시엔 넘겨 받은 V.A의 포맷에서 VPN을 추출해 
해당 Value를 Index로 하여 Page table 탐색

변환 정보가 있다면, PFN을 그대로 추출해 Physical Address 도출해 동작 수행.
없다면? Page fault 발생.
1)특정 Reg ( CR2 Reg )에 저장되어 있는 Faulting Address를 추출.
2)해당 Address가 포함된 Virtual Page가 Valid한지 확인 
= 해당 Page에 대한 PTE가 있는지 확인. 
A. PTE있으면 Disk->memory로 swap in 하고 
업데이트된 frame을 해당 page에 다시 Mapping해 Page Table 업데이트.
이후 다시 명령 수행
B. 만약, PTE가 없다면? 이땐 Faulting Address가 Stack Segment가 
확장했을 때 Cover할 수 있는 영역 내에 존재하는지 확인한다. 
Growable Region에 해당하는지를 보는 것임.
➔ 해당한다면? 후술할 Stack Growth를 수행하면 된다.
➔ Growable Region에 포함되지 않는다면? 이는 Segmentation Fault에 해당한다. 즉, Process를
죽이고 Free해야 한다. 
//////////////////////////////////////


3. stack growth?
*/
////////////
#include "vm/page.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"


/* helper functions */
static unsigned pt_hash (const struct hash_elem *h_elem, void *UNUSED);
static bool pt_cmp (const struct hash_elem *left, const struct hash_elem *right, void *UNUSED);
static void pt_destroy_func (struct hash_elem *h_elem, void *UNUSED);

extern struct lock frame_lock;
/*
lock_acquire(&frame_lock);
lock_release(&frame_lock);


*/
void 
pt_init (struct hash *pt)//완
{
  hash_init (pt, pt_hash, pt_cmp, NULL);
}

void 
pt_destroy (struct hash *pt)//완
{
  
  lock_acquire(&frame_lock);
  hash_destroy (pt, pt_destroy_func);
  lock_release(&frame_lock);

}

bool //0
pt_insert_entry (struct hash *pt, struct pt_entry *pte)
{
  lock_acquire(&frame_lock);
  bool success=hash_insert(pt, &(pte->elem));
  lock_release(&frame_lock);
  return success;
}

bool //완
pt_delete_entry (struct hash *pt, struct pt_entry *pte)
{
  lock_acquire(&frame_lock);
  bool success=hash_delete(pt, &(pte->elem));
  lock_release(&frame_lock);
  if (!success) return false;
  free_page (pagedir_get_page (thread_current ()->pagedir, pte->vaddr));
  free (pte);
  return true;
}

struct pt_entry *
pt_find_entry (void *vaddr)//완
{
  struct pt_entry tmp; 
  struct pt_entry *ve = NULL;
  struct hash_elem *elem;
  tmp.vaddr = pg_round_down(vaddr); //malloc?
  elem = hash_find(&thread_current()->pt, &tmp.elem);
  if(elem) ve = hash_entry(elem, struct pt_entry, elem);
  return ve;//NULL처리?
}


static unsigned//완
pt_hash (const struct hash_elem *h_elem, void *aux UNUSED)
{
  struct pt_entry *pte = hash_entry(h_elem, struct pt_entry, elem);

  return hash_int ((int)(pte->vaddr));
}

static bool//완
pt_cmp (const struct hash_elem *A, 
  const struct hash_elem *B, void *aux UNUSED)
{
  struct pt_entry *a = hash_entry(A, struct pt_entry, elem);
  struct pt_entry *b = hash_entry(B, struct pt_entry, elem);

  return ((a->vaddr) < (b->vaddr));
}

void page_delete(struct pt_entry *pte){//완
    void *paddr = pagedir_get_page(thread_current()->pagedir, pte->vaddr);
    free_page(paddr); //=frame_free
}
static void pt_destroy_func(struct hash_elem *e, void *aux UNUSED){//완
    struct pt_entry *ve = hash_entry(e, struct pt_entry, elem);

    if(ve->is_loaded){
      page_delete(ve);
    }
    free(ve);
}
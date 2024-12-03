// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
  // char* ref_page;
  // int page_cnt;
  // char* end_;
} kmem;

#define PA2PGREF_ID(p) (((p)-KERNBASE)/PGSIZE)
#define PGREF_MAX_ENTRIES PA2PGREF_ID(PHYSTOP)
struct spinlock pgreflock;
int pageref[PGREF_MAX_ENTRIES];

#define PA2PGREF(p) pageref[PA2PGREF_ID((uint64)(p))]


// int pagecnt(void *pa_start, void *pa_end){
//   char *p;
//   int cnt = 0;
//   p = (char*)PGROUNDUP((uint64)pa_start);
//   for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
//     cnt++;
//   return cnt;
// }

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&pgreflock, "pgref");
  freerange(end, (void*)PHYSTOP);
}

// int page_index(uint64 pa){
//   pa = PGROUNDDOWN(pa);
//   int res = (pa - (uint64) kmem.end_)/PGSIZE;
//   if(res < 0 || res >= kmem.page_cnt){
//     panic("page_index illegal");
//   }
//   return res;
// }

// void
// incr(void* pa){
//   int index = page_index((uint64)pa);
//   acquire(&kmem.lock);
//   kmem.ref_page[index]++;
//   release(&kmem.lock);
// }

// void desc(void* pa){
//   int index = page_index((uint64)pa);
//   acquire(&kmem.lock);
//   kmem.ref_page[index]--;
//   release(&kmem.lock);
// }

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // int index = page_index((uint64)pa);
  // if(kmem.ref_page[index] > 1){
  //   desc(pa);
  //   return ;
  // }
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&pgreflock);
  if(--PA2PGREF(pa) <= 0){

    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  release(&pgreflock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r)
    memset((char*)r, 5, PGSIZE); // fill with junk
    PA2PGREF(r) = 1;
  //incr(r);
  return (void*)r;
}

void *kcopy_n_deref(void* pa){
  acquire(&pgreflock);

  if(PA2PGREF(pa) <= 1){
    release(&pgreflock);
    return pa;
  }

  uint64 newpa = (uint64)kalloc();
  if(newpa == 0){
    release(&pgreflock);
    return 0;
  }
  memmove((void*)newpa, (void*)pa, PGSIZE);

  PA2PGREF(pa)--;

  release(&pgreflock);
  return (void*)newpa;
}

void krefpage(void* pa){
  acquire(&pgreflock);
  PA2PGREF(pa)++;
  release(&pgreflock);
}

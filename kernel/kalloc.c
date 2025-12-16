// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

/* We will store a reference count for each physical page here: */
static int ref_count[(PHYSTOP - KERNBASE) / PGSIZE];

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

/* Convert physical address to index in reference count array */
static inline int
pa2idx(uint64 pa)
{
  return (pa - KERNBASE) / PGSIZE;
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");

  for(int i = 0; i < (PHYSTOP - KERNBASE) / PGSIZE; i++) {
    ref_count[i] = 0;
  }
  
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  acquire(&kmem.lock);
  
  // Decrement reference count
  int idx = pa2idx((uint64)pa);
  ref_count[idx]--;
  
  // Only free if reference count reaches 0
  if(ref_count[idx] > 0) {
    release(&kmem.lock);
    return;
  }

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

 // acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
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

  if(r) {
    memset((char*)r, 5, PGSIZE);
    // Only set ref_count if we got a valid page
    int idx = pa2idx((uint64)r);
    if(idx >= 0 && idx < (PHYSTOP - KERNBASE) / PGSIZE) {
      ref_count[idx] = 1;
    }
  }

  return (void*)r;
}

uint64 
Kfreepages(void)
{
  uint64 number_of_pages = 0;
  struct run *r;
  
  acquire(&kmem.lock);
  r = kmem.freelist;
  while(r)
  {
    number_of_pages++;
    r = r->next;
  }
  release(&kmem.lock);
  
  return number_of_pages;
}

uint64 
freemem(void)
{
 uint64 free_pages = Kfreepages();
 return (free_pages * PGSIZE) / 1024;
}

//increments the reference count
void
krefpage(void *pa)
{  
  acquire(&kmem.lock);
  ref_count[pa2idx((uint64)pa)]++;
  release(&kmem.lock);
}

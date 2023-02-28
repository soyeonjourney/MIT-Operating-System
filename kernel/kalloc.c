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
} kmem;

struct {
  struct spinlock lock;
  int array[MAXREFINDEX];
} refcount;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&refcount.lock, "refcount");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    acquire(&refcount.lock);
    refcount.array[PG2REFINDEX(p)] = 1;
    release(&refcount.lock);
    kfree(p);
  }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;
  int rfidx, rfcnt;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&refcount.lock);
  rfidx = PG2REFINDEX(pa);
  if(refcount.array[rfidx] > 0){
    refcount.array[rfidx]--;
    rfcnt = refcount.array[rfidx];
  } else {
    panic("kfree: refcount is 0");
  }
  release(&refcount.lock);

  if(rfcnt == 0){
    // Fill with junk to catch dangling refs.
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
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
  if(r){
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // fill with junk
    acquire(&refcount.lock);
    if(refcount.array[PG2REFINDEX(r)] > 0)
      panic("kalloc: refcount is not 0");
    refcount.array[PG2REFINDEX(r)] = 1;
    release(&refcount.lock);
  }
  return (void*)r;
}

// API for refcount increment
void
krefinc(void *pa)
{
  acquire(&refcount.lock);
  refcount.array[PG2REFINDEX(pa)]++;
  release(&refcount.lock);
}

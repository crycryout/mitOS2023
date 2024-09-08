// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NHASH 27

struct {
  struct spinlock lock;
  struct spinlock hashlocks[NHASH];
  struct spinlock buflocks[NBUF];
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  //struct buf head;
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  
  for (int i = 0; i < NHASH; i++) {
    initlock(&bcache.hashlocks[i], "bcache");
  }

  for (int i = 0; i < NBUF; i++) {
    initlock(&bcache.buflocks[i], "buflocks");
  }

  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;

  int hash = blockno % NHASH;
  acquire(&bcache.hashlocks[hash]);

  // Is the block already cached?
  for (int i = 0; i < NBUF; i++) {
    b = &bcache.buf[i];
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release(&bcache.hashlocks[hash]);
      acquiresleep(&b->lock);
      return b;
    } 
  }
  release(&bcache.hashlocks[hash]);
  // Not cached.
  for (int i = 0 ;i < NBUF;i++) { 
    acquire(&bcache.hashlocks[hash]);
    acquire(&bcache.buflocks[i]);
    b = &bcache.buf[i];
    if(b->refcnt == 0) {
      b->dev = dev;
      b->blockno = blockno;
      b->valid = 0;
      b->refcnt = 1;
      release(&bcache.buflocks[i]);
      release(&bcache.hashlocks[hash]);
      acquiresleep(&b->lock);
      return b;
    }
    release(&bcache.buflocks[i]);
    release(&bcache.hashlocks[hash]);
  }
  panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf *b)
{
  if(!holdingsleep(&b->lock))
    panic("bwrite");
  virtio_disk_rw(b, 1);
}

// Release a locked buffer.
// Move to the head of the most-recently-used list.
void
brelse(struct buf *b)
{
  int hash = b->blockno % NHASH;
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  acquire(&bcache.hashlocks[hash]);
  b->refcnt--;
  //if (b->refcnt == 0) {
    // no one is waiting for it.
    //b->next->prev = b->prev;
    //b->prev->next = b->next;
    //b->next = bcache.head.next;
    //b->prev = &bcache.head;
    //bcache.head.next->prev = b;
    //bcache.head.next = b;
  //}
  
  release(&bcache.hashlocks[hash]);
}

void
bpin(struct buf *b) {
  int hash = b->blockno % NHASH;
  acquire(&bcache.hashlocks[hash]);
  b->refcnt++;
  release(&bcache.hashlocks[hash]);
}

void
bunpin(struct buf *b) {
  int hash = b->blockno % NHASH;
  acquire(&bcache.hashlocks[hash]);
  b->refcnt--;
  release(&bcache.hashlocks[hash]);
}



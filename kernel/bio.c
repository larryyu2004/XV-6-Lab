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
//TODO, Buffer cache
#define NBUC 13
//TODO, Buffer cache

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  //TODO, Buffer cache
  //struct buf head;
  //TODO, Buffer cache
} bcache;

//TODO, Buffer cache
struct bMem{
  struct spinlock lock;
  struct buf head;
};
static struct bMem hashTable[NBUC];
//TODO, Buffer cache

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");
  //TODO, Buffer cache
  // Create linked list of buffers
  for(int i = 0; i < NBUC; i++){
    initlock(&(hashTable[i].lock), "bcache.bucket");
  }
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    initsleeplock(&b->lock, "buffer");
  }
  //TODO, Buffer cache
}

//TODO, Buffer cache
void replaceBuffer(struct buf *lrubuf, uint dev, uint blockno, uint ticks){
  lrubuf->dev = dev;
  lrubuf->blockno = blockno;
  lrubuf->valid = 0;
  lrubuf->refcnt = 1;
  lrubuf->tick = ticks;
}
//TODO, Buffer cache

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  //TODO, Buffer cache
  struct buf *lastBuf;
  
  // Is the block already cached?
  uint64 num = blockno%NBUC;
  acquire(&(hashTable[num].lock));
  for(b = hashTable[num].head.next, lastBuf = &(hashTable[num].head); b; b = b->next){
    if(!(b->next)){
      lastBuf = b;
    }
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      release((&hashTable[num].lock));
      acquiresleep(&b->lock);
      return b;
    }
  }

  struct buf *lruBuf = 0;
  acquire(&bcache.lock);
  for(b = bcache.buf; b < bcache.buf + NBUF; b++){
    if(b->refcnt == 0){
      if(lruBuf == 0){
        lruBuf == 0;
        continue;
      }
      if(b->tick < lruBuf->tick){
        // Update if current buffer is older (lower tick value)
        lruBuf = b;
      }
    }
  }

  if(lruBuf){
    uint64 oldTick = lruBuf->tick;
    uint64 oldNum = (lruBuf->blockno)%NBUC;
    if(oldTick == 0){
      //If the old buffer was never accessed (tick == 0), directly replace it.
      replaceBuffer(lruBuf, dev, blockno, ticks);
      lastBuf->next = lruBuf;
      lruBuf->prev = lastBuf;
    }else{
      if(oldNum != num){
        //If the buffer belongs to a different hash table bucket:
        acquire(&(hashTable[oldNum].lock));
        replaceBuffer(lruBuf, dev, blockno, ticks);
        //Remove the buffer from the old linked list
        lruBuf->prev->next = lruBuf->next;
        if(lruBuf->next){
          lruBuf->next->prev = lruBuf->prev;
        }
        release(&(hashTable[oldNum].lock));
        lastBuf->next = lruBuf;
        lruBuf->prev = lastBuf;
        lruBuf->next = 0;
      }else{
        //If the buffer already belongs to the correct hash table bucket, replace it directly.
        replaceBuffer(lruBuf, dev, blockno, ticks);
      }
    }
    release(&(bcache.lock));
    release(&(hashTable[num].lock));
    acquiresleep(&lruBuf->lock);
    return lruBuf;
  }
  //TODO, Buffer cache
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
  if(!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);
  //TODO, Buffer cache
  uint64 num = b->blockno%NBUC;
  acquire(&(hashTable[num].lock));
  b->refcnt--; 
  release(&(hashTable[num].lock));
  //TODO, Buffer cache
}

void
bpin(struct buf *b) {
  //TODO, Buffer cache
  uint64 num = b->blockno%NBUC;
  acquire(&(hashTable[num].lock));
  b->refcnt++;
  release(&(hashTable[num].lock));
  //TODO, Buffer cache
}

void
bunpin(struct buf *b) {
  //TODO, Buffer cache
  uint64 num = b->blockno%NBUC;
  acquire(&(hashTable[num].lock));
  b->refcnt--;
  release(&(hashTable[num].lock));
  //TODO, Buffer cache
}


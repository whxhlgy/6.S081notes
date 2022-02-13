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

#define NBUCKET 13

struct {
  struct spinlock lock;
  struct buf buf[NBUF];

  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
  // struct buf head;
  struct buf hashtable[NBUCKET];
  struct spinlock bucket_lock[NBUCKET];
} bcache;

void
binit(void)
{
  struct buf *b;

  initlock(&bcache.lock, "bcache");

  // Create linked list of buffers
  int i;
  for (i = 0; i < NBUCKET; i++) {
    initlock(&bcache.bucket_lock[i], "bcache.bucket");
    bcache.hashtable[i].prev = &bcache.hashtable[i];
    bcache.hashtable[i].next = &bcache.hashtable[i];
  }
  for(b = bcache.buf; b < bcache.buf+NBUF; b++){
    b->next = bcache.hashtable[0].next;
    b->prev = &bcache.hashtable[0];
    initsleeplock(&b->lock, "buffer");
    bcache.hashtable[0].next->prev = b;
    bcache.hashtable[0].next = b;
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf *b;
  int i;
  extern uint ticks;

  // acquire(&bcache.lock);

  int key = blockno % NBUCKET;
  // Is the block already cached?
  acquire(&bcache.bucket_lock[key]); // 获取key对应的桶锁，该锁直到最后才放开
  for(b = bcache.hashtable[key].next; b != &bcache.hashtable[key]; b = b->next){
    if(b->dev == dev && b->blockno == blockno){
      b->refcnt++;
      b->stamp = ticks;
      release(&bcache.bucket_lock[key]);
      // release(&bcache.lock);
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // Recycle the least recently used (LRU) unused buffer.
  struct buf *lru = 0;
  acquire(&bcache.lock);
  for (i = 0; i < NBUCKET; i++) {
    for(b = bcache.hashtable[i].next; b != &bcache.hashtable[i]; b = b->next){
      if (lru == 0 && b->refcnt == 0)
        lru = b;
      if(b->refcnt == 0 && b->stamp < lru->stamp) {
        lru = b;
      }
    }
  }
  if (lru == 0) 
    panic("bget: no buffers");

  // eviction
  lru->next->prev = lru->prev;
  lru->prev->next = lru->next;

  // 加入新的bucket
  lru->next = bcache.hashtable[key].next;
  lru->prev = &bcache.hashtable[key];
  bcache.hashtable[key].next->prev = lru;
  bcache.hashtable[key].next = lru;

  b = lru;
  b->dev = dev;
  b->blockno = blockno;
  b->valid = 0;
  b->refcnt = 1;
  b->stamp = ticks;
  release(&bcache.lock);
  release(&bcache.bucket_lock[key]); 
  acquiresleep(&b->lock);
  return b;
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

  int key = b->blockno % NBUCKET;
  // acquire(&bcache.lock);
  acquire(&bcache.bucket_lock[key]);
  b->refcnt--;
  release(&bcache.bucket_lock[key]);
  // release(&bcache.lock);
}

void
bpin(struct buf *b) {
  int key = b->blockno % NBUCKET;
  // acquire(&bcache.lock);
  acquire(&bcache.bucket_lock[key]);
  b->refcnt++;
  release(&bcache.bucket_lock[key]);
  // release(&bcache.lock);
}

void
bunpin(struct buf *b) {
  int key = b->blockno % NBUCKET;
  // acquire(&bcache.lock);
  acquire(&bcache.bucket_lock[key]);
  b->refcnt--;
  release(&bcache.bucket_lock[key]);
  // release(&bcache.lock);
}



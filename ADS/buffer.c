#define NBUF 64
#define NDEV 12

typedef struct buf {
    struct buf *next_free; // freelist pointer
    struct buf *next_dev; // dev_list pointer
    int dev,blk; // assigned disk block;
    int opcode; // READ|WRITE
    int dirty; // buffer data modified
    int async; // ASYNC write flag
    int valid; // buffer data valid
    int busy; // buffer is in use
    int wanted; // some process needs this buffer
    struct semaphore lock=1; // buffer locking semaphore; value=1
    struct semaphore iodone=0; // for process to wait for I/O completion;
    char buf[BLKSIZE]; // block data area
} BUFFER;

BUFFER buf[NBUF]; // NBUF buffers

// assume all bufs are initially free
struct freelist {
    BUFFER* first;
    BUFFER* last;
    int size = NBUF;
    struct semaphore lock = NBUF;
} FREELIST;

struct devtab{
 u16 dev; // major device number
 BUFFER *dev_list; // device buffer list
 BUFFER *io_queue; // device I/O queue
} DEVTABLE;

DEVTABLE devtable[NDEV];

BUFFER* getblk(int dev, int blk)
{
    // DISABLE INTERRUPTS
    // (1) search for (dev, blk) in devtable[dev]
    // (2) if (dev, blk) in devtable[dev]
    //      2.1) bp = (dev, blk) in devtable[dev]
    //      2.2) P(&bp->lock)
    //      2.3) if (bp in freelist) take it out
    // (3) else
    //      3.1) P(freelist->lock)
    //      3.2) bp = dequeue freelist
    //      3.3) P(bp->lock)
    //      3.4) if (bp DIRTY) write out bp
    //      3.5) enter bp into devtable[dev]
    // (4) return bp
    // ENABLE INTERRUPTS
}

void brelse(BUFFER* bp)
{
    // DISABLE INTERRUPTS
    // (1) if (bp->lock.value == 0)
    //      1.1) V(freelist)
    //      1.2) enter bp into end of freelist
    // (2) V(bp->lock)
    // ENABLE INTERRUTPS
}
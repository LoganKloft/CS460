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
typedef struct freelist {
    BUFFER* first;
    BUFFER* last;
    int size = NBUF;
    struct semaphore lock = NBUF;
} FREELIST;

typedef struct devtab{
 u16 dev; // major device number
 BUFFER *dev_list; // device buffer list
 BUFFER *io_queue; // device I/O queue
} DEVTABLE;

typedef struct process {
    // ...
    // tells us that this process wants a block from a specific device
    int wanted_dev; // -1 for none
    int wanted_block; // -1 for none
    // ...
} PROCESS;

DEVTABLE devtable[NDEV];

BUFFER* getblk(int dev, int blk)
{
    // DISABLE INTERRUPTS - explanation in summary section
    // <! potential deadlock check here - explained at end !>
    // (1) if (dev, blk) in devtable[dev]
    //      1.1) bp = (dev, blk) in devtable[dev]
    //      1.2) P(&bp->lock)
    //      1.3) if (bp in freelist) take it out
    // (2) if (dev, blk) in freelist's semaphore's sleep list (wanted_dev, wanted_block)
    //      2.1) P(freelist->lock)
    //      2.2) bp = (dev, blk) in devtable[dev]
    //      2.3) P(&bp->lock)
    // (3) else
    //      3.1) running->wanted_dev = dev, running->wanted_blk = blk 
    //      3.2) P(freelist->lock)
    //      3.3) bp = dequeue freelist
    //      3.4) P(bp->lock)
    //      3.5) if (bp DIRTY) write out bp
    //      3.6) enter bp into devtable[dev]
    //      (4)  for process in freelist semaphore sleep list
    //          4.1) if current (dev, iblk) == process (wanted_dev, wanted_blk)
    //          4.2) remove process from freelist's semaphore's sleep list, increment freelist semaphore value
    //          4.3) add removed process to buffer's semaphore's sleep list, decrement buffer semaphore value
    // (5) return bp
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
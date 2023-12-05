typedef struct process {
    // ...
    BUFFER* bp_channel; // store received buffer
    // ...
} PROCESS

BUFFER* getblk(int dev, int blk)
{
    DISABLE INTERRUPTS
    getblk_helper(dev, blk);
    bp = running->bp_channel;
    running->bp_channel = 0;
    return bp;
    ENABLE INTERRUPTS
}

void getblk_helper(int dev, int blk)
{
    // we want buffer that is already available
    if (bp in dev_list)
    {
        P(bp->lock)

        // we didn't have to wait for a brelse
        if (running->bp_channel == null) 
        {
            remove buffer from freelist
            running->bp_channel = bp in dev_list
        }
    }
    
    // we want a buffer that is not already available
    // guaranteed we will have to 'make' our own buffer
    else
    {
        P(freelist->available)

        // we didn't have to wait for a brelse
        if (running->bp_channel == null)
        {
            bp = dequeue from freelist
            enter bp into dev_list with updated dev, blk, and remove from old devlist
            running->bp_channel = bp
            P(bp->lock) // will now be 0
        }

        // we had to wait for a brelse
        else
        {
            // there could be a chance that another process(es)
            // was also waiting for the same dev, blk
            // for each one we find in the freelist, we will
            // remove it from the freelist and add to bp's
            // lock list
            for proc in freelist->available.procs
            {
                if proc.dev == dev and proc.blk == blk
                {
                    p = remove(&freelist->available.proc, proc)
                    freelist->available.value++

                    add(&bp->lock.proc, p)
                    bp->lock.value--
                }
            }
        }
    }
}

void brelse(BUFFER* bp)
{
    DISABLE INTERRUPTS
    // nobody waiting for this specific buffer
    if (bp->lock.value == 0)
    {
        // somebody waiting for buffer from freelist
        if (freelist->available.value < 0)
        {
            p = first process in available queue

            // so that way a process with higher priority
            // can't steal this bp, reduce cache effect here
            // cache should still be reasonable
            remove bp from devlist
            
            if (bp->dirty)
            {
                // remove bp from devlist to because
                // we are going to repurpose it for our
                // own dev, blk
                write bp out

                // we assume only go here after write out
                // finishes so buffer is not dirty anymore
                // in other words, write bp out causes
                // this process to sleep

                // additionally, if we don't want this process
                // to sleep, we can enforce a constraint that
                // write out will be in charge of waking up
                // the process from the freelist
            }

            // bp might have been written out - dirty
            // or it might not have - not dirty
            enter bp into dev_list with updated dev, blk
            p->bp_channel = bp;
            V(freelist->available) // wake up process that
                                   // we assigned bp to
        }

        // nobody waiting for buffer at all
        else
        {
            enter bp into freelist

            V(freelist->available) // won't wake anyone
            V(bp->lock) // won't wake anyone up

            // after: available.value >= 1
            // after: lock.value == 1
        }
    }

    // somebody waiting for this buffer
    else
    {
        // we assign this buffer to first process in lock queue
        p = first process in bp lock queue
        p->bp_channel = bp;

        // we wake up buffer, when awoken, no other process
        // will sneak before this one in accessing the buffer
        // since they would have been put in the lock queue
        V(buffer->lock)

        // buffer.value <= 0, thus no process sneak in
    }
    ENABLE INTERRUPTS
}
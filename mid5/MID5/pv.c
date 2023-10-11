int block(struct semaphore *s)
{
    running->status = BLOCK;
    running->next = 0;
    enqueue_fifo(&s->queue, running);
    kprintf("P%d> blocked in P()\n", running->pid);
    printList("BLOCK:", s->queue);
    tswitch();
}

int signal(struct semaphore *s)
{
    PROC *p = dequeue(&s->queue);
    p->status = READY;
    kprintf("P%d> unblocked P%d in V()\n", running->pid, p->pid);
    enqueue(&readyQueue, p);
}

int P(struct semaphore *s)
{
    int SR = int_off();

    s->value--;
    if (s->value < 0)
    {
        block(s);
    }
    int_on(SR);

    return 1;
}

int V(struct semaphore *s)
{
    int SR = int_off();

    s->value++;
    if (s->value <= 0)
    {
        signal(s);
    }

    int_on(SR);
}
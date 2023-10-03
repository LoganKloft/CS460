# define NPIPE 8

PIPE pipe[NPIPE];
int pipezize = sizeof(PIPE);

int pipe_init()
{
    kprintf("pipe_init()\n");
    int i;
    PIPE* p;
    for(i = 0; i < NPIPE; i++)
    {
        p = &pipe[i];
        p->head = 0;
        p->tail = 0;
        p->data = 0;
        p->room = PSIZE;
        p->status = FREE;
        p->nreader = 0;
        p->nwriter = 0;

        p->w_sema.value = 0;
        p->w_sema.queue = 0;
        p->r_sema.value = 0;
        p->r_sema.queue = 0;
    }
}

PIPE* create_pipe()
{
    int i;
    PIPE* p;
    for (i = 0; i < NPIPE; i++)
    {
        p = &pipe[i];

        if (p->status == FREE)
        {
            p->status = READY;
            p->nreader = 1;
            p->nwriter = 1;
            return p;
        }
    }

    return 0;
}

int destroy_pipe(PIPE *p)
{
    p->head = 0;
    p->tail = 0;
    p->data = 0;
    p->room = PSIZE;
    p->status = FREE;
    p->nreader = 0;
    p->nwriter = 0;

    while (p->w_sema.queue)
    {
        PROC *pr = dequeue(&p->w_sema.queue);
        pr->status = READY;
        enqueue(&readyQueue, pr);
    }

    while (p->r_sema.queue)
    {
        PROC *pr = dequeue(&p->r_sema.queue);
        pr->status = READY;
        enqueue(&readyQueue, pr);
    }

    p->w_sema.value = 0;
    p->w_sema.queue = 0;
    p->r_sema.value = 0;
    p->r_sema.queue = 0;
}

int read_pipe(PIPE *p, char *buf, int n)
{
    int r = 0;
    if (n <= 0)
        return 0;

    if (p->status == FREE) // p->status must not be FREE
        return 0;

    while (n)
    {
        // read n data or until there is no data left
        while (p->data)
        {
            *buf++ = p->buf[p->tail++]; // read a byte to buf
            p->tail %= PSIZE;
            p->data--; p->room++; r++; n--;
            if (n==0)
                break;
        }

        V(&p->w_sema);

        if (r) // if has read some data
            return r;

        // pipe has no data 

        if (p->nwriter == 0)
        {
            // pipe has no writer
            kprintf("pipe has no writer AND no data: return 0\n");
            return 0;
        }

        P(&p->r_sema);
    }
}

int write_pipe(PIPE *p, char *buf, int n)
{ 
    int r = 0;

    if (n <= 0)
        return 0;

    if (p->status == FREE) // p->status must not be FREE
        return 0;

    while (n)
    {
        if (p->nreader == 0)
        {
            // no writer left
            printf("BROKEN PIPE\n");
            return -1;
        }

        // write n data or until there is no room left
        while (p->room)
        {
            p->buf[p->head++] = *buf++; // write a byte to pipe;
            p->head %= PSIZE;
            p->data++; p->room--; r++; n--;

            if (n == 0)
                break;
        }

        V(&p->r_sema);

        if (n == 0)
            return r; // finished writing n bytes

        // still has data to write but pipe has no room
        // V(&p->r_sema);
        P(&p->w_sema); // sleep for room
    }
}
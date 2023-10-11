# define NPIPE 8

PIPE pipes[NPIPE];
int pipezize = sizeof(PIPE);

int pipe_init()
{
    kprintf("pipe_init()\n");
    int i;
    PIPE* p;
    for(i = 0; i < NPIPE; i++)
    {
        p = &pipes[i];
        p->head = 0;
        p->tail = 0;
        p->status = FREE;
        p->nreader = 0;
        p->nwriter = 0;

        p->data.value = 0;
        p->data.queue = 0;
        p->room.value = PSIZE;
        p->room.queue = 0;
    }
}

PIPE* create_pipe()
{
    int i;
    PIPE* p;
    for (i = 0; i < NPIPE; i++)
    {
        p = &pipes[i];

        if (p->status == FREE)
        {
            p->head = 0;
            p->tail = 0;
            p->status = READY;
            p->data.value = 0;
            p->data.queue = 0;
            p->room.value = PSIZE;
            p->room.queue = 0;
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
    p->data.value = 0;
    p->room.value = PSIZE;
    p->status = FREE;
    p->nreader = 0;
    p->nwriter = 0;

    while (p->data.queue)
    {
        PROC *pr = dequeue(&p->data.queue);
        pr->status = READY;
        enqueue(&readyQueue, pr);
    }

    while (p->room.queue)
    {
        PROC *pr = dequeue(&p->room.queue);
        pr->status = READY;
        enqueue(&readyQueue, pr);
    }

    p->data.queue = 0;
    p->room.queue = 0;
}

int read_pipe(PIPE *p, char *buf, int n)
{
    int r = 0;
    if (n <= 0)
        return 0;

    if (p->status == FREE) // p->status must not be FREE
        return 0;
    
    if (p->nwriter == 0 && p->data.value == 0)
    {
        // pipe has no writer and no data
        kprintf("pipe has no writer AND no data: return 0\n");
        return 0;
    }

    while (n)
    {
        // read n data or until there is no data left
        while (P(&p->data))
        {
            if (p->nwriter == 0 && p->data.value == 0)
            {
                // pipe has no writer and no data
                kprintf("pipe has no writer AND no data: return 0\n");
                return 0;
            }

            *buf++ = p->buf[p->tail++]; // read a byte to buf
            p->tail %= PSIZE;
            V(&p->room); r++; n--;
            if (n==0)
                break;
        }

        if (r) // if has read some data
            return r;
    }
}

int write_pipe(PIPE *p, char *buf, int n)
{ 
    int r = 0;

    if (n <= 0)
        return 0;

    if (p->status == FREE) // p->status must not be FREE
        return 0;

    if (p->nreader == 0)
    {
        // no reader left
        printf("BROKEN PIPE\n");
        return -1;
    }

    while (n)
    {
        // write n data or until there is no room left
        while (P(&p->room))
        {
            if (p->nreader == 0)
            {
                // no reader left
                printf("BROKEN PIPE\n");
                return -1;
            }

            p->buf[p->head++] = *buf++; // write a byte to pipe;
            p->head %= PSIZE;
            V(&p->data); r++; n--;
            
            if (n == 0)
                break;
        }

        if (n == 0)
            return r; // finished writing n bytes
    }
}
int *pfreeList; // free page frame list

int *palloc() // allocate a free page frame
{
    int *p = pfreeList;
    if (p){
        pfreeList = (int *)(*p);
        *p = 0;
    }
    return p;
}
void pdealloc(int p) // release a page frame
{
    u32 *a = (u32 *)((int)p & 0xFFFFF000); // frame's PA
    *a = (int)pfreeList;
    pfreeList = a;
}
int *free_page_list(int *startva, int *endva) // build pfreeList
{
    int *p = startva;

    while(p < (int *)(endva-1024)){
        *p = (int)(p + 1024);
        p += 1024;
    }

    *p = 0;

    return startva;
}

int init_memory()
{
    pfreeList = free_page_list((int *) (0x100000 * 8), (int *) (0x100000 * 256));
}
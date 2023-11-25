#include "ucode.c"

int main(int argc, char* argv[])
{
    char buf[BLKSIZE];
    int fd;
    int gd;

    if (argc == 3)
    {
        fd = open(argv[1], O_RDONLY);
        gd = open(argv[2], O_WRONLY | O_CREAT);

        if (fd < 0 || gd < 0)
        {
            printf("Cp failed to open file(s) for READ/WRITE\n");
            return -1;
        }
    }

    // invalid call to cp
    else
    {
        return -1;
    }

    int r;
    while ((r = read(fd, buf, BLKSIZE)) > 0)
    {
        write(gd, buf, r);
    }
}
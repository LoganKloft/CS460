#include "ucode.c"

int main(int argc, char* argv[])
{
    char buf[BLKSIZE + 1];
    int fd;

    // (1) reading from stdin
    if (argc == 1)
    {
        fd = 0;
        char* cp = buf;
        int r;
        while ((r = read(fd, cp, 1)) > 0)
        {
            if (*cp == '\n' || *cp == '\r')
            {
                write(1, "\n\r", 2);
                write(1, buf, cp - buf);
                write(1, "\n\r", 2);
                cp = buf;
            }
            else
            {
                write(1, cp, 1);
                cp++;
            }
        }
    }

    // (2) reading from file
    else
    {
        fd = open(argv[1], O_RDONLY);
        if (fd < 0)
        {
            printf("Cat failed to open file for READ\n");
            return -1;
        }

        int r;
        while ((r = read(fd, buf, BLKSIZE)) > 0)
        {
            buf[r] = 0;
            write(1, buf, r);
        }

        close(fd);
    }
}
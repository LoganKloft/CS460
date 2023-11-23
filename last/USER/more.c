#include "ucode.c"

int main(int argc, char* argv[])
{
    int fd;
    char buf[BLKSIZE];
    int r;

    // (1) from stdin
    if (argc == 1)
    {
        fd = 0;
    }

    // (2) from argv[1]
    else
    {
        fd = open(argv[1], O_RDONLY);

        if (fd < 0)
        {
            printf("More failed to open file for READ\n");
        }

        // we pretend user pressed space first
        int space = 1;
        while ((r = read(fd, buf, BLKSIZE)) > 0)
        {
            char* cp = buf;
            char* stop = buf + r;
            int lines_read = 0;

            while (cp != stop)
            {
                // if 'enter' show 1 line
                if (space == 0)
                {

                }

                // if 'space' show 25 lines
                else if (space == 1)
                {

                }

                // we need to get more characters
                if (cp == stop) break;

                // else get input => 'enter' == '\r', 'space' == ' '
                char c = getc();
                if (c == ' ') space = 1;
                else if (c == '\r') space = 0;
                else space = -1;
            }

            // nothing left to read
            if (stop != buf + BLKSIZE) break;
        }

        close(fd);
    }
}
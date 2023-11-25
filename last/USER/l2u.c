#include "ucode.c"

int main(int argc, char* argv[])
{
    char buf[BLKSIZE];
    int fd; // input
    int gd; // output
    int lowerToUpper = 'A' - 'a';

    if (argc == 1)
    {
        fd = 0;
        gd = 1;
    }
    else if (argc == 3)
    {
        fd = open(argv[1], O_RDONLY);
        gd = open(argv[1], O_WRONLY | O_CREAT);

        if (fd < 0 || gd < 0)
        {
            printf("L2u failed to open file(s) for READ/WRITE\n");
            return -1;
        }
    }
    else
    {
        return -1;
    }

    STAT st_stdin;
    fstat(fd, &st_stdin);
    int from_pipe_or_file = 0;
    if (S_ISFIFO(st_stdin.st_mode) || S_ISREG(st_stdin.st_mode))
    {
        from_pipe_or_file = 1;
    }

    STAT st_stdout;
    fstat(gd, &st_stdout);
    int to_pipe_or_file = 0;
    if (S_ISFIFO(st_stdout.st_mode) || S_ISREG(st_stdout.st_mode))
    {
        to_pipe_or_file = 1;
    }

    if (from_pipe_or_file)
    {
        int r;
        while ((r = read(fd, buf, BLKSIZE)) > 0)
        {
            if (to_pipe_or_file)
            {
                for (int i = 0; i < r; i++)
                {
                    // convert to uppercase
                    if (buf[i] >= 'a' && buf[i] <= 'z')
                    {
                        buf[i] = buf[i] + lowerToUpper;
                    }
                    
                    write(gd, &buf[i], 1);
                }
            }
            else
            {
                for (int i = 0; i < r; i++)
                {
                    if (buf[i] == '\n')
                    {
                        write(gd, "\n\r", 2);
                    }
                    else
                    {
                        // convert to uppercase
                        if (buf[i] >= 'a' && buf[i] <= 'z')
                        {
                            buf[i] = buf[i] + lowerToUpper;
                        }

                        write(gd, &buf[i], 1);
                    }
                }
            }
        }
    }
    
    // no double echo like cat and more
    else
    {
        int r;
        while ((r = read(fd, buf, 1)) > 0)
        {
            if (to_pipe_or_file)
            {
                for (int i = 0; i < r; i++)
                {
                    if (buf[i] == '\r')
                    {
                        write(gd, "\n", 1);
                    }
                    else
                    {
                        // convert to uppercase
                        if (buf[i] >= 'a' && buf[i] <= 'z')
                        {
                            buf[i] = buf[i] + lowerToUpper;
                        }

                        write(gd, &buf[i], 1);
                    }
                }
            }
            else
            {
                for (int i = 0; i < r; i++)
                {
                    if (buf[i] == '\r')
                    {
                        write(gd, "\n\r", 2);
                    }
                    else
                    {
                        // convert to uppercase
                        if (buf[i] >= 'a' && buf[i] <= 'z')
                        {
                            buf[i] = buf[i] + lowerToUpper;
                        }

                        write(gd, &buf[i], 1);
                    }
                }
            }
        }
    }
}
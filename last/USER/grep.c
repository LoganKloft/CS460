#include "ucode.c"

int main(int argc, char* argv[])
{
    int fd;
    char buf[BLKSIZE];
    char* pattern;

    // (0) not enough arguments command
    if (argc == 1)
    {
        return -1;
    }

    // (1) from stdin
    if (argc == 2)
    {
        fd = 0;
    }

    // (2) from file
    if (argc == 3)
    {
        fd = open(argv[2], O_RDONLY);
        if (fd < 0) printf("Grep failed to opepn file for READ\n");
    }

    // pattern is second argument regardless of where reading from
    pattern = argv[1];

    // (3) in source
    STAT st_stdin;
    fstat(fd, &st_stdin);
    int from_pipe_or_file = 0;
    if (S_ISFIFO(st_stdin.st_mode) || S_ISREG(st_stdin.st_mode))
    {
        from_pipe_or_file = 1;
    }

    // (4) out destination
    STAT st_stdout;
    fstat(1, &st_stdout);
    int to_pipe_or_file = 0;
    if (S_ISFIFO(st_stdout.st_mode) || S_ISREG(st_stdout.st_mode))
    {
        to_pipe_or_file = 1;
    }

    // (5) handle from pipe or file
    if (from_pipe_or_file)
    {
        char line[81];
        char line_index = 0;
        int r;
        char *cp = buf;
        char *stop = buf;

        // Algorithm
        // fill buffer if all characters used
        // read line by line
        // a line is 80 characters or '\n'
        // make line a string
        // if line contains pattern print it

        while (1)
        {
            if (cp == stop)
            {
                r = read(fd, buf, BLKSIZE);

                if (r < 1) break;

                cp = buf;
                stop = buf + r;
            }

            while (cp < stop)
            {
                if (*cp == '\n' || line_index == 80)
                {
                    int found = 0;
                    line[line_index] = '\0';
                    if ((mystrstr(line, pattern)) != 0)
                    {
                        found = 1;
                    }

                    if (found)
                    {
                        if (to_pipe_or_file)
                        {
                            write(1, line, line_index);
                            write(1, "\n", 1);
                        }
                        else
                        {
                            write(1, line, line_index);
                            write(1, "\n\r", 2);
                        }
                    }

                    line_index = 0;
                }
                else
                {
                    line[line_index] = *cp;
                    line_index++;
                }

                cp++;
            }
        }
    }

    // (6) handle from user
    // same as cat but we only mirror input
    // if it contains the pattern
    else
    {
        char *cp = buf;
        int r;
        while ((r = read(fd, cp, 1)) > 0)
        {
            // write line-by-line to output
            if (*cp == '\r')
            {
                // check if buf contains target
                *cp = '\0';
                int found = 0;
                if ((mystrstr(buf, pattern)) != 0)
                {
                    found = 1;
                }

                if (to_pipe_or_file)
                {
                    if (found)
                    {
                        // replace '\0' with '\n'
                        *cp = '\n';
                        cp++; // otherwise wouldn't include '\n'
                        write(1, buf, cp - buf);
                    }

                    // then move cursor to next line
                    write(2, "\n\r", 2);
                }
                else
                {
                    // we must go to new line
                    write(1, "\n\r", 2);

                    if (found)
                    {
                        // we must print copy of input
                        write(1, buf, cp - buf);

                        // we must go to new line again
                        write(1, "\n\r", 2);
                    }
                }

                // reset buffer
                cp = buf;
            }

            // read character-by-character
            else
            {
                // so user can see input
                write(2, cp, 1); 
                cp++;
            }
        }
    }
}
#include "ucode.c"

// we assume 80 characters wide
int main(int argc, char* argv[])
{
    int fd;
    char buf[BLKSIZE];
    char tty[128];
    gettty(tty);
    int gd = open(tty, O_RDONLY);

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
    }

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

    // (5) handle in from pipe or file - 'interesting' part
    if (from_pipe_or_file)
    {
        int lines_read = 0;
        int lines_needed = 25;
        char line[80];
        int line_index = 0;
        int r;
        char* cp = buf;
        char* stop = buf;
        
        while (1)
        {
            if (cp == stop)
            {
                r = read(fd, buf, BLKSIZE);

                // nothing left to read
                if (r < 1) break;

                cp = buf;
                stop = buf + r;
            }

            if (lines_read == lines_needed)
            {
                while(1)
                {
                    char c;
                    read(gd, &c, 1);
                    if (c == ' ') {
                        lines_read = 0;
                        lines_needed = 25;
                        break;
                    }
                    else if (c == '\r') {
                        lines_read = 0;
                        lines_needed = 1;
                        break;
                    }
                }
            }

            while (lines_read < lines_needed)
            {
                // we get 80 characters 25 times
                // if we see '\n' then this ends the line
                while (cp < stop)
                {
                    if (*cp == '\n' || line_index == 80)
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

                        lines_read++;
                        line_index = 0;
                    }
                    else
                    {
                        line[line_index] = *cp;
                        line_index++;
                    }

                    cp++;

                    if (lines_read == lines_needed) break;
                }

                // we need to get more characters
                if (cp == stop) break;
            }
        }
    }

    // (6) handle in from user - LITERAL COPY OF CAT
    // realistically, more does not process user input
    // so we must define our own behavior, it makes the most
    // sense to mimic cat, the example code does not
    // seem to provide a useful implementation
    else
    {
        char *cp = buf;
        int r;
        while ((r = read(fd, cp, 1)) > 0)
        {
            // write line-by-line to output
            if (*cp == '\r')
            {
                if (to_pipe_or_file)
                {
                    // replace '\r' with '\n'
                    *cp = '\n';
                    cp++; // otherwise wouldn't include '\n'
                    write(1, buf, cp - buf);

                    // then move cursor to next line
                    write(2, "\n\r", 2);
                }
                else
                {
                    // we must go to new line
                    write(1, "\n\r", 2);

                    // we must print copy of input
                    write(1, buf, cp - buf);

                    // we must go to new line again
                    write(1, "\n\r", 2);
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
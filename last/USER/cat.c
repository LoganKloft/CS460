#include "ucode.c"

int main(int argc, char* argv[])
{
    char buf[BLKSIZE];
    int fd;

    // (1) reading pipe, user input, or input redirection
    if (argc == 1) {
        fd = 0;
    }
    // (2) reading opened file
    else {
        fd = open(argv[1], O_RDONLY);
    }

    // stat stdin
    // if type is not pipe or file we are reading user input
    // for user input, when we receive a '\r' we must print a 
    // '\n'
    STAT st_stdin;
    fstat(fd, &st_stdin);
    int from_pipe_or_file = 0;
    if (S_ISFIFO(st_stdin.st_mode) || S_ISREG(st_stdin.st_mode))
    {
        from_pipe_or_file = 1;
    }

    // stat stdout
    // if type is pipe or file, then we write '\n' for
    // encountered '\r' or '\n'
    STAT st_stdout;
    fstat(1, &st_stdout);
    int to_pipe_or_file = 0;
    if (S_ISFIFO(st_stdout.st_mode) || S_ISREG(st_stdout.st_mode))
    {
        to_pipe_or_file = 1;
    }

    // pipe or file input
    if (from_pipe_or_file)
    {
        int r;
        while ((r = read(fd, buf, BLKSIZE)) > 0)
        {
            if (to_pipe_or_file)
            {
                write(1, buf, r);
            }
            else
            {
                for (int i = 0; i < r; i++)
                {
                    if (buf[i] == '\n')
                    {
                        write(1, "\n\r", 2);
                    }
                    else
                    {
                        write(1, &buf[i], 1);
                    }
                }
            }
        }
    }

    // user input
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
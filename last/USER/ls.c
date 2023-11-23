#include "ucode.c"

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";
int ls_file(char* fname)
{
    STAT st;
    stat(fname, &st);

    // print file type
    if (S_ISDIR(st.st_mode))
    {
        printf("%c", 'd');
    }
    else if(S_ISREG(st.st_mode))
    {
        printf("%c", '-');
    }
    else if(S_ISLNK(st.st_mode))
    {
        printf("%c", 'l');
    }

    // print file permissions
    for (int i = 8; i >= 0; i--)
    {
        if (st.st_mode & (1 << i))
        {
            printf("%c", t1[i]);
        }
        else
        {
            printf("%c", t2[i]);
        }
    }

    // print user and group information
    printf(" %d\t", st.st_nlink); // link count
    printf("%d\t", st.st_uid);    // uid
    printf("%d\t", st.st_gid);    // gid
    printf("%d\t", st.st_size);   // file size

    char bname[128];
    basename(fname, bname);
    printf("%s", bname);

    char lname[128];
    if (S_ISLNK(st.st_mode))
    {
        readlink(fname, lname);
        printf(" -> %s", lname);
    }

    printf("\n\r");
}

int ls_dir(char *dname)
{
    // (1) open dname for mode read
    int fd = open(dname, O_RDONLY);
    char buf[BLKSIZE];
    read(fd, buf, BLKSIZE);

    // (2) ls_file all entries
    DIR* dirent = buf;
    char* cp = buf;
    char name[256];

    int i = 0;
    while (cp != buf + BLKSIZE)
    {
        strcpy(name, dname);
        strcat(name, "/");
        strncpy(name + strlen(name), dirent->name, dirent->name_len);
        ls_file(name);
        cp += dirent->rec_len;
        dirent = cp;
        bzero(name, 256);
    }

    close(fd);
}

int main(int argc, char* argv[])
{
    char filename[BLKSIZE];
    char cwd[BLKSIZE];
    STAT st;
    char* s;

    // (1) process arguments
    if (argc == 1)
    {
        s = "./";
    }
    else
    {
        s = argv[1];
    }

    // (2) build filename
    if ((stat(s, &st)) < 0)
    {
        printf("File does not exist\n");
        uexit();
    }

    strcpy(filename, s);
    if (s[0] != '/')
    {
        getcwd(cwd);
        strcpy(filename, cwd);
        strcat(filename, "/");
        strcat(filename, s);
    }

    // (3) handle file
    if (S_ISDIR(st.st_mode))
    {
        ls_dir(filename);
    }

    // (4) handle directory
    else
    {
        ls_file(filename);
    }
}
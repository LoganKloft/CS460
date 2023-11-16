
typedef struct ext2_dir_entry_2 {
	u32	inode;			/* Inode number */
	u16	rec_len;		/* Directory entry length */
	u8	name_len;		/* Name length */
	u8	file_type;
	char	name[255];      	/* File name */
} DIR;

typedef struct stat {
  u16    st_dev;		/* major/minor device number */
  u16    st_ino;		/* i-node number */
  u16    st_mode;		/* file mode, protection bits, etc. */
  u16    st_nlink;		/* # links; TEMPORARY HACK: should be nlink_t*/
  u16    st_uid;			/* uid of the file's owner */
  u16    st_gid;		/* gid; TEMPORARY HACK: should be gid_t */
  u16    st_rdev;
  long   st_size;		/* file size */
  long   st_atime;		/* time of last access */
  long   st_mtime;		// time of last modification
  long   st_ctime;		// time of creation
  long   st_dtime;
  long   st_date;
  long   st_time;
} STAT;

// UNIX <fcntl.h> constants: <asm/fcntl.h> in Linux
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02
#define O_CREAT		   0100	/* not fcntl */
#define O_TRUNC		  01000	/* not fcntl */
#define O_APPEND	  02000

#define EOF  -1

#define exit mexit


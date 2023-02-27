#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <stdlib.h>

typedef struct _dirdesc {
    int dd_fd;
    long dd_loc;
    long dd_size;
    char *dd_buf;
    int dd_len;
    off_t dd_seek;
    void *dd_lock;
    int dd_magic;
} DIRDESC;

typedef struct my_dirent {
    ino_t d_ino;
    off_t d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char

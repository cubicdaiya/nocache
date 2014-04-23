#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define TRUE  1
#define FALSE 0

void exiterr(const char *s)
{
    perror(s);
    exit(-1);
}

void usage(void)
{
    fprintf(stderr, "usage: cachedel [-n <n>] [-r <r>] -f <file> "
            "-- call fadvise(DONTNEED) <n> times on file\n");
}

int run(char *fn, int n, double r)
{
    int i, fd;
    struct stat st;
    off_t l;

    fd = open(fn, O_RDONLY);

    if(fd == -1) {
        exiterr("open");
    }

    if(fstat(fd, &st) == -1) {
        exiterr("fstat");
    }

    if(!S_ISREG(st.st_mode)) {
        fprintf(stderr, "%s: S_ISREG: not a regular file", fn);
        return FALSE;
    }

    if(st.st_size == 0) {
        fprintf(stderr, "%s: file size is 0!\n", fn);
        return FALSE;
    }

    if (r < 0.0 || r > 1.0) {
        fprintf(stderr, "%f: rate should be less than 1.0\n", r);
        return FALSE;
    }

    l = (off_t)(st.st_size * r);

    for(i = 0; i < n; i++) {
        if(posix_fadvise(fd, 0, l, POSIX_FADV_DONTNEED) == -1) {
            exiterr("posix_fadvise");
        }
    }

    return TRUE;
}

int main(int argc, char *argv[])
{
    int n, c;
    char *fn;
    double r;

    n = 1;
    r = 0;

    while (-1 != (c = getopt(argc, argv,
                             "n:" /* <number> will repeat the syscall */
                             "r:" /* <rate> will flush page cache rate */
                             "f:" /* file */
                             "h"  /* show help */
                             )))
    {
        switch (c) {
        case 'n':
            n = atoi(optarg);
            break;
        case 'r':
            r = atof(optarg);
            break;
        case 'f':
            fn = malloc(strlen(optarg) + 1);
            memcpy(fn, optarg, strlen(optarg));
            break;
        case 'h':
            usage();
            return 0;
            break;
        default:
            break;
        }
    }

    run(fn, n, r);

    free(fn);

    return EXIT_SUCCESS;
}

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fts.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define setbit(b, n) ((b) |= (1 << (n)))
#define readbit(b, n) ((b) & (1 << (n)))

enum options
{
    optA,
    optS,
    optP,
    optD,
    optX,
    optU
};
typedef struct
{
    char name[256];
    long long size;
    short depth;
    unsigned char flags;
    int index;
    int desc;

} entry;

void invalid_use();
int cmp_name(const FTSENT **ent1, const FTSENT **ent2);
int check_optDecimal(const char *str, long int *value);
void print_size(long size, long root_size, char opt_flags);
void indent(unsigned char flags, short depth, char *ind_flag);
int get_options(char *opt_flags, long int *depth, int argc, char *argv[]);
entry *sort_by_size(entry *array, int size);
entry *sort_folder(entry *array, entry *arr_s, entry *arr_final, int index, int);
int size_compar(const void *a, const void *b);
void print_single(struct stat st, char opt_flags);

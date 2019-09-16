#include "utils.h"

void invalid_use()
{
    fprintf(stderr, "a.out: invalid arguments\nUsage: a.out [-asp] [-d NUMBER] PATH\n");
}

// getting user supplied options
int get_options(char *opt_flags, long int *depth, int argc, char *argv[])
{
    int opt = 0;
    if (argc < 2) {
        invalid_use();
        return -1;
    }
    while ((opt = getopt(argc, argv, "asd:pxu")) != -1) {
        switch (opt) {
        case 'a':
            if (readbit(*opt_flags, optA)) {
                invalid_use();
                return -1;
            }
            setbit(*opt_flags, optA);
            break;
        case 's':
            if (readbit(*opt_flags, optS)) {
                invalid_use();
                return -1;
            }
            setbit(*opt_flags, optS);
            break;
        case 'p':
            if (readbit(*opt_flags, optP)) {
                invalid_use();
                return -1;
            }
            setbit(*opt_flags, optP);
            break;
        case 'd':
            if (check_optDecimal(optarg, depth) != 0)
                return -1;
            if (readbit(*opt_flags, optD)) {
                invalid_use();
                return -1;
            }
            setbit(*opt_flags, optD);
            break;
        case 'x':
            if (readbit(*opt_flags, optX)) {
                invalid_use();
                return -1;
            }
            setbit(*opt_flags, optX);
            break;
        case 'u':
            if (readbit(*opt_flags, optU)) {
                invalid_use();
                return -1;
            }
            setbit(*opt_flags, optS);
            break;
        default:
            invalid_use();
            return -1;
        }
    }
    if (argv[optind] == NULL) {
        fprintf(stderr, "a.out: invalid arguments, missing PATH\nUsage: a.out [-asp] [-d NUMBER] PATH\n");
        return -1;
    }

    return 0;
}

// check if arg NUMBER after -d option is positiv integer
int check_optDecimal(const char *str, long int *value)
{
    errno = 0;
    char *endptr;
    *value = strtol(str, &endptr, 10);

    if (*value < 0L || (*value == LONG_MAX && errno == ERANGE)) {
        fprintf(stderr, "a.out: invalid option argument NUMBER\n\
		  Must be non-negative integer\n\
		  Usage: a.out [-asp] [-d NUMBER] PATH\n");
        return -1;
    }
    if (str == endptr || *endptr != '\0') {
        fprintf(stderr, "a.out: invalid option argument NUMBER\nUsage: a.out [-asp] [-d NUMBER] PATH\n");
        return -1;
    }
    return 0;
}
//string to lowercase
char *strlow(char *str)
{
    unsigned char *c = (unsigned char *) str;
    while (*c) {
        *c = tolower((unsigned char) *c);
        c++;
    }

    return str;
}
// printing size of tree entity
void print_size(long size, long root_size, char opt_flags)
{
    if (readbit(opt_flags, optP)) {
        printf("%5.1f%% ", (1000 * size / root_size) / 10.0);
        return;
    }
    if (size < 1024)
        printf("%6.1f B   ", (float) size);
    else if (size < 1048576)
        printf("%6.1f KiB ", (size * 10 / 1024) / 10.0);
    else if (size < 1073741824)
        printf("%6.1f MiB ", (size * 10 / 1048576) / 10.0);
    else if (size < 1099511627776)
        printf("%6.1f GiB ", (size * 10 / 1073741824) / 10.0);
    else if (size < 1125899906842624)
        printf("%6.1f TiB ", (size * 10 / 1099511627776) / 10.0);
    else if (size < 1152921504606846976)
        printf("%6.1f PiB ", (size * 10 / 1125899906842624) / 10.0);
    else
        fprintf(stderr, "size overflow\n");
}

// dealing with the indentation when printing tree structure
void indent(unsigned char flags, short depth, char *ind_flag)
{
    if (readbit(flags, 0))
        return;

    for (int i = 1; i < depth; i++) {
        if (ind_flag[i] == 0)
            printf("|   ");
        else
            printf("    ");
    }

    if (readbit(flags, 2))
        printf("\\-- ");
    else
        printf("|-- ");
}

// compar func for fts_open
int cmp_name(const FTSENT **ent1, const FTSENT **ent2)
{
    int result = strcasecmp((*ent1)->fts_name, (*ent2)->fts_name);
    if (result == 0)
        result = strcmp((*ent1)->fts_name, (*ent2)->fts_name);
    return result;
}

// sorting tree by size
// I'm quite sure it's not in nlogn because of the arr_final copying
// thou I wasn't able to solve it with passing pointer

entry *sort_by_size(entry *array, int size)
{
    entry *arr_s = malloc(sizeof(entry) * size);
    entry *arr_final = malloc(sizeof(entry) * size);
    arr_final[size - 1].depth = -1;
    arr_final[0] = array[0];
    for (int inx_f = 0; inx_f < size - 1; inx_f++) {
        if (arr_final[inx_f].desc < 2)
            continue;
        arr_final = sort_folder(array, arr_s, arr_final, arr_final[inx_f].index + 1, inx_f + 1);
    }
    free(arr_s);
    free(array);
    return arr_final;
}

// sorting subdirectory for sorting by size
entry *sort_folder(entry *array, entry *arr_s, entry *arr_final, int index, int index_final)
{
    int inx_s = 0;
    entry entity = array[index];
    while (array[entity.index + entity.desc].depth == entity.depth) {
        arr_s[inx_s] = entity;
        entity = array[entity.index + entity.desc];
        inx_s++;
    }
    arr_s[inx_s] = entity;
    qsort(arr_s, inx_s + 1, sizeof(entry), &size_compar);
    int inx_f = 0;
    for (int i = 0; i < inx_s; i++) {
        arr_final[index_final + inx_f] = arr_s[i];
        inx_f += arr_s[i].desc;
    }
    arr_final[index_final + inx_f] = arr_s[inx_s];
    setbit(arr_final[index_final + inx_f].flags, 2);
    return arr_final;
}

// compar func for qsort of sorting by size
int size_compar(const void *a, const void *b)
{
    if ((long long) ((entry *) a)->size > ((long long) ((entry *) b)->size))
        return -1;
    if ((long long) ((entry *) a)->size < ((long long) ((entry *) b)->size))
        return 1;
    int result = strcasecmp(((entry *) a)->name, ((entry *) b)->name);
    if (result == 0)
        result = strcmp(((entry *) a)->name, ((entry *) b)->name);
    return result;
}

// printing when user supplied arg PATH is not a directory
// tree traversal is not able to handle that situation
void print_single(struct stat st, char opt_flags)
{
    long long f_size = st.st_size;
    if (readbit(opt_flags, optA))
        f_size = st.st_blocks * 512;
    print_size(f_size, f_size, opt_flags);
}

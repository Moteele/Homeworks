#include "utils.h"

int main(int argc, char *argv[])
{
    /*
     * this code really is a clusterfuck, despite all my intention
     * lessons learned:
     *	- always read instruction properly before getting to work
     *	    (addresing -s option)
     *	- don't use lib functions unless you are totally sure with that
     *	    (adressing fts.h)
     *	- exceptions are the worst part of progrramming
     */
    long int depth = -1;
    FTS *ftsp = NULL;
    FTSENT *root = NULL;
    int fts_flags = FTS_PHYSICAL;
    FTSENT *entity = NULL;
    int error_occured = 0;
    short max_depth = 1;
    char opt_flags = 0;
    char root_name[4096];
    struct stat st = { 0 };
    int (*compar)(const FTSENT **, const FTSENT **) = &cmp_name;

    if (get_options(&opt_flags, &depth, argc, argv) != 0)
        return -1;

    // check PATH exists
    if (stat(argv[optind], &st)) {
        perror(argv[optind]);
        return -1;
    }
    if (S_ISREG(st.st_mode) || S_ISLNK(st.st_mode)) {
        print_single(st, opt_flags);
        printf("%s\n", argv[optind]);
        return 0;
    }
    if ( ! S_ISDIR(st.st_mode))
	return 0;

    if (readbit(opt_flags, optX))
        fts_flags |= FTS_XDEV;

    // dont let fts_open sort tree if user wants to sort by size
    if (readbit(opt_flags, optS))
        compar = NULL;

    // needed add slash for unknown reason
    if (!strcmp(argv[optind], ".")) {
        char *path[] = { "./", NULL };
        ftsp = fts_open(path, fts_flags, compar);
    }

    else if (!strcmp(argv[optind], "..")) {
        char *path[] = { "../", NULL };
        ftsp = fts_open(path, fts_flags, compar);
    }

    else
        ftsp = fts_open(&argv[optind], fts_flags, compar);

    if (ftsp == NULL) {
        perror(argv[optind]);
        return -1;
    }

    // checking PATH in a directory

    root = fts_children(ftsp, 0);

    if (root == NULL) {
        fts_close(ftsp);
        fprintf(stderr, "PATH is neither directory or file, aborting");
        return -1;
    }

    if (root->fts_info != FTS_D) {
        fts_close(ftsp);
        fprintf(stderr, "PATH is neither directory or file, aborting");
        return -1;
    }

    entry *array;
    int num = 0;

    /*
     * first time tree traversal for counting the tree size
     * I belive it might be faster than dynamic allocation in most cases
     */

    while ((entity = fts_read(ftsp)) != NULL) {
        if ((root == entity) && (entity->fts_info == FTS_DP)) {
            strncpy(root_name, entity->fts_path, sizeof(root_name));
            fts_set(ftsp, entity, FTS_AGAIN);
            num++;
            break;
        }
        switch (entity->fts_info) {
        case FTS_D:
            break;
        case FTS_DEFAULT:
	    break;
        case FTS_ERR:
        case FTS_DNR:
            num++;
            error_occured = 1;
            perror(entity->fts_name);
            break;
        case FTS_DP:
            num++;
            break;
        case FTS_NS:
            error_occured = 1;
        // fall
        case FTS_SL:
        case FTS_F:
            num++;
            break;
        default:
            fprintf(stderr, "err: unknown file type");
            break;
        }
    }

    // allocating tree array
    if ((array = malloc(sizeof(entry) * (num + 1))) == NULL) {
        fprintf(stderr, "Can't allocate resources");
        return -1;
    }

    // clang still warns flags might be uninitialized
    // I wonder why?
    array[num].depth = -1;
    for (int i = 0; i < num; i++) {
        array[i].flags = 0;
    }
    int index = 0;

    // traversing the tree second time
    // saving all the important info
    while ((entity = fts_read(ftsp)) != NULL) {
        if (index > num) {
            fprintf(stderr, "index > num\n");
            free(array);
            return -1;
        }
        if (entity->fts_level > max_depth)
            max_depth = entity->fts_level;
        switch (entity->fts_info) {
        case FTS_D: // directory visit preorder
            strncpy(array[index].name, entity->fts_name, 255);
            entity->fts_pointer = (entry *) &array[index];
            setbit(array[index].flags, 1);
            array[index].depth = entity->fts_level;
            array[index].index = index;
            index++;
            break;
        case FTS_DNR: // directory visit error
            setbit(((entry *) entity->fts_pointer)->flags, 3);
            // fall
        case FTS_DP: // directory visit postorder
            if (readbit(opt_flags, optA))
                entity->fts_number += entity->fts_statp->st_size;
            else
                entity->fts_number += entity->fts_statp->st_blocks * 512;
            ((entry *) entity->fts_pointer)->size = entity->fts_number;
            if (entity == root) {
                break;
            }
            entity->fts_parent->fts_number += entity->fts_number;
            if (readbit(((entry *) entity->fts_pointer)->flags, 3))
                setbit(((entry *) entity->fts_parent->fts_pointer)->flags, 3);
            ((entry *) entity->fts_pointer)->desc = index - ((entry *) entity->fts_pointer)->index;

            break;

        case FTS_SL: // symlink
        case FTS_F:  // file visit
            if (readbit(opt_flags, optA))
                entity->fts_number += entity->fts_statp->st_size;
            else
                entity->fts_number += entity->fts_statp->st_blocks * 512;
            entity->fts_parent->fts_number += entity->fts_number;
            strncpy(array[index].name, entity->fts_name, 255);
            array[index].size = entity->fts_number;
            array[index].depth = entity->fts_level;
            array[index].index = index;
            array[index].desc = 1;
            index++;
            break;

        case FTS_ERR:
            perror(entity->fts_name);
            free(array);
            return -1;
            break;
        case FTS_NS: // file with no stat info
            perror(entity->fts_name);
            setbit(array[index].flags, 3);
            setbit(((entry *) entity->fts_parent->fts_pointer)->flags, 3);
            array[index].index = index;
            array[index].desc = 1;
            index++;
            break;
        default:
            break;
        }
    }

    setbit(array[0].flags, 0);
    strncpy(array[0].name, root_name, 255);
    // sorting by size
    if (readbit(opt_flags, optS)) {
        array[0].desc = num;
        array = sort_by_size(array, num + 1);
    }
    else {
	for (int i = 0; i < num; i++) {
	    if (array[i].desc < 2)
		continue;
	    int x = i + 1;
	    while (array[array[x].desc + i].depth == array[i + 1].depth)
		x += array[x].desc;
	    //x += array[x].desc - 1;
	    setbit(array[x].flags, 2);
	}
    }
    char ind_flag[max_depth];
    memset(ind_flag, 0, max_depth);

    // printing the tree
    for (int i = 0; i < num; i++) {

        if (readbit(opt_flags, optD) && array[i].depth > depth)
            continue;
        if (readbit(array[i].flags, 1)) {
            if ((readbit(array[i].flags, 2)))
                ind_flag[array[i].depth] = 1;
            else
                ind_flag[array[i].depth] = 0;
        }
        if (error_occured) {
            if (readbit(array[i].flags, 3))
                printf("? ");
            else
                printf("  ");
        }

        print_size(array[i].size, array[0].size, opt_flags);
        indent(array[i].flags, array[i].depth, ind_flag);
        printf("%s\n", array[i].name);
    }

    free(array);
    fts_close(ftsp);

    return 0;
}




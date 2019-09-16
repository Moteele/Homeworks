#include "md5.h"
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int test_hash(unsigned char *hash, unsigned char *result, char *passwd);

int find_transformations(int **array, char *string);
void make_hash(char *line, int nread, unsigned char *result, MD5_CTX *context);
int combine(unsigned char *hash, unsigned char *result, char *passwd,\
	ssize_t *nread, int **array, MD5_CTX *context, short index);

int main(int argc, char *argv[])
{
    FILE *file;
    int opt = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char opt_t = 0;
    unsigned char result[16];
    unsigned char hash[16];
    int perms;
    MD5_CTX context;

    if (argc < 3 ||  argc > 4) {
        fprintf(stderr, "invalid use\nUSAGE: ./cracker [tc] FILE HASH\n");
        return -1;
    }
    while ((opt = getopt(argc, argv, "t")) != -1) {
        switch (opt) {
        case 't':
            opt_t = 1;
            break;
        //case 'c':
	//  break;
        default:
            fprintf(stderr, "invalid option\n");
            return -1;
        }
    }
    if (argv[optind] == NULL) {
        fprintf(stderr, "missing FILE\nUSAGE: ./cracker [tc] FILE HASH\n");
        return -1;
    }
    int hash_p = optind + 1;
    if (argv[hash_p] == NULL) {
        fprintf(stderr, "missing HASH\nUSAGE: ./cracker [tc] FILE HASH\n");
        return -1;
    }
    if (strlen(argv[hash_p]) != 32) {
        fprintf(stderr, "hash is not of correct length\n");
        return -1;
    }

    // checking file exists and could be read
    file = fopen(argv[optind], "r");
    if (file == NULL) {
        perror(argv[optind]);
        return -1;
    }
    char x[3];
    x[2] = '\0';
    char *endptr = &x[0];
    // interpreting user-supplied hash in form md5 library uses
    for (int i = 0; i < 32; i += 2) {
        x[0] = argv[hash_p][i];
        x[1] = argv[hash_p][i + 1];
        hash[i / 2] = strtol(x, &endptr, 16);
        if (endptr != &x[2]) {
            fprintf(stderr, "hash is not in correct format\n");
            fclose(file);
            return -1;
        }
    }
    int arr_size = 20;
    int *array = malloc(sizeof(int) * arr_size);
    if (array == NULL)
        return -1;
    int *array_old = array;

    // iterates over each line in file
    while ((nread = getline(&line, &len, file)) != -1) {
        if (opt_t) {
            if ((int) nread >= arr_size) {
                arr_size *= 2;
                array = realloc(array, sizeof(int) * arr_size);
                if (array == NULL) {
                    free(array_old);
                    return -1;
                }
            }
            array_old = array;

            perms = find_transformations(&array, line);
            if (perms > 1) {
                if (combine(hash, result, line, &nread, &array, &context, 0))
                    break;
            }
        } else {
	    make_hash(line, nread, result, &context);
	    if (test_hash(hash, result, line))
		break;
	}
    }

    if (nread == -1)
        printf("password not found\n");
    fclose(file);
    free(array);
    free(line);
    return 0;
}

// hashes the string
void make_hash(char *line, int nread, unsigned char *result, MD5_CTX *context)
{
    MD5_Init(context);
    MD5_Update(context, line, nread - 1);
    MD5_Final(result, context);
}
// tests the computed hash against the user-supplied one
int test_hash(unsigned char *hash, unsigned char *result, char *passwd)
{
    for (int i = 0; i < 16; i++) {
        if (result[i] != hash[i])
            break;
        if (i != 15)
            continue;
        printf("password found\n%s", passwd);
        return 1;
    }
    return 0;
}
// finds all symbols that could be transformed
// used for optimization, so that each recursive call of combine()
// doesnt have to iterate over the string
int find_transformations(int **array, char *string)
{
    int index = 0;
    // the number of permutations is not really used
    // but might be useful for rewriting combine() in iterative approach
    int permutations = 1;
    for (int i = 0; string[i] != '\0'; i++) {
        switch (tolower(string[i])) {
        case 'a':
        case 's':
            (*array)[index] = i;
            permutations *= 3;
            break;
        case 'b':
        case 'e':
        case 'i':
        case 'l':
        case 'o':
        case 't':
            (*array)[index] = i;
            permutations *= 2;
            break;
        default:
            continue;
        }
        index++; // does not increment on switch default case
    }
    (*array)[index] = -1;
    return permutations;
}
// recursively tests all iterations
// the depth of recursion is O(n) where n is the number of transformable chars
// computational complexity should be of O(3^n)
// iterative approach would be more effective, but i dismissed it because 
// some chars have more than one transformation, so it couldnt be solved simply
// by binary masking
int combine(unsigned char *hash, unsigned char *result, char *passwd,\
	ssize_t *nread, int **array, MD5_CTX *context, short index)
{
    if ((*array)[index] == -1) {
        make_hash(passwd, *nread, result, context);
        if (test_hash(hash, result, passwd))
            return 1;
	return 0;
    }
    int inx = (*array)[index];
    char ch = passwd[inx];
    // a lot of repetiton here
    // maybe i should have used macros for this one?
    // or gotos, as switch already is a goto function?
    switch (tolower(ch)) {
    case 'a':
        passwd[inx] = '@';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        passwd[inx] = '4';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 's':
        passwd[inx] = '$';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        passwd[inx] = '5';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 'b':
        passwd[inx] = '8';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 'e':
        passwd[inx] = '3';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 'i':
        passwd[inx] = '!';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 'l':
        passwd[inx] = '1';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 'o':
        passwd[inx] = '0';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    case 't':
        passwd[inx] = '7';
        if (combine(hash, result, passwd, nread, array, context, index + 1))
            return 1;
        break;
    }
    passwd[inx] = ch;
    if (combine(hash, result, passwd, nread, array, context, index + 1))
        return 1;
    return 0;
}

/*
 * Aidan Bird 2021
 * a1birdATryersonDOTca
 *
 * COLUMNIZER
 * Formats strings into columns.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define ERROR_MSG_BUF_SIZE 69

#define ARGS_ERROR  \
{ \
    printErrorMsg("Invalid option '%s'\n", argv[i]); \
    exit(1); \
}

#define EXPECTED_CHAR_ERROR \
{ \
    printErrorMsg("Expected character, got '%s'\n", argv[i]); \
    exit(1); \
}

#define TRY_GET_CHAR(SRC_PTR, DEST_PTR) \
if (isStrAChar(args)) { \
    *(DEST_PTR) = *(SRC_PTR); \
} else { \
    EXPECTED_CHAR_ERROR; \
}

typedef enum ArgsLoopState ArgsLoopState;

enum ArgsLoopState
{
    START,
    ROW_SEPARATOR,
    COL_SEPARATOR,
    DELIMITER,
};

int isStrAChar(const char *str);
void printErrorMsg(const char *fmt, ...);
size_t sumStrLen(size_t n, const char **strs);
size_t parseRow(char *restrict buf, char *col, char **nextcol, char delim);
int printCol(size_t n, char **strs, char rowSeparator, char colSeparator,
    char delim);

static const char *usage = 
"USAGE 1: columnizer [OPTIONS]... [STRINGS]...\n"
"USAGE 2: columnizer -- [STRINGS]...\n"
"Formats strings into columns.\n"
"\n"
"OPTIONS\n"
"  -h               Prints this help message.\n"
"  --               Indicates that there are no more \n"
"                   option arguments after --.\n"
"  -d [CHARACTER]   Sets the input delimiter character. Space is default.\n"
"                   The delimiter cannot be NULL (zero).\n"
"  -r [CHARACTER]   Sets the output row separator character.\n"
"                   Newline is default.\n"
"  -c [CHARACTER]   Sets the output column separator character.\n"
"                   Space is default.\n"
"\n"
"EXAMPLE\n"
"columnizer -d \" \" -r \"\\n\" -c \" \" -- \"a b c\" \"1 2 3\"\n"
"a 1\n"
"b 2\n"
"c 3\n"
"\n"
"Aidan Bird 2021";

/*
 * REQUIRES
 * valid fmt
 *
 * MODIFIES
 * stderr
 *
 * EFFECTS
 * prints an error message.
 */
void
printErrorMsg(const char *fmt, ...)
{
    static char buffer[ERROR_MSG_BUF_SIZE];
    va_list args;

    va_start (args, fmt);
    /*
     * if the message is too large, it will be truncated and ... will be
     * appended to it
     */
    if (vsnprintf(buffer, ERROR_MSG_BUF_SIZE, fmt, args) 
        > ERROR_MSG_BUF_SIZE) {
        buffer[ERROR_MSG_BUF_SIZE - 2] = '.';  
        buffer[ERROR_MSG_BUF_SIZE - 3] = '.';
        buffer[ERROR_MSG_BUF_SIZE - 4] = '.';
        buffer[ERROR_MSG_BUF_SIZE - 5] = ' ';  
    }
    fprintf(stderr, "columnizer: %s", buffer);
    va_end (args);
}

/*
 * REQUIRES
 * none
 *
 * MODIFIES
 * none
 *
 * EFFECTS
 * return non-zero when str is not null and had only one non-null character
 * followed by the null terminator.
 */
int
isStrAChar(const char *str)
{
    return str && str[0] && !str[1];
}

/*
 * REQUIRES
 * correct n and valid strs
 *
 * MODIFIES
 * none
 *
 * EFFECTS
 * returns the maximum strlen of n strs
 */
size_t
sumStrLen(size_t n, const char **strs)
{
    size_t ret;

    ret = 0;
    for (int i = 0; i < n; i++)
        ret += strlen(strs[i]);
    return ret;
}

/*
 * REQUIRES
 * buf can store the result
 *
 * MODIFIES
 * buf, col
 *
 * EFFECTS
 * All col mutations are temporary, col shall remain the same before and after 
 * this function ends.
 * The result is pasted into buf (Beware of buffer overruns!).
 * The length of the result is returned.
 */
size_t
parseRow(char *restrict buf, char *col, char **nextcol, char delim)
{
    size_t ret;
    char *start;
    char isModCol;

    start = col;
    if (!start)
        return 0;
    while (*start && *start == delim)
        start++;
    if (!*start)
        return 0;
    col = strchr(start, delim);
    isModCol = 0;
    if (col) {
        *col = '\0';
        isModCol = 1;
    }
    strcpy(buf, start);
    ret = strlen(start);
    if (isModCol)
        *col = delim;
    if (nextcol)
        *nextcol = start + ret +isModCol;
    return ret;
}

/*
 * REQUIRES
 * valid n and strs
 *
 * MODIFIES
 * strs, stdout
 *
 * EFFECTS
 * prints strs arranged in columns. 
 * each column is separated by a space.
 * rows are separated by newlines.
 * returns non-zero on error.
 */
int
printCol(size_t n, char **strs, char rowSeparator, char colSeparator,
    char delim)
{
    int j;
    char *buf;
    size_t lenSum;
    size_t nCopied;
    size_t lastReadSize;

    if (!n)
        return 0;
    lenSum = sumStrLen(n, strs);
    buf = malloc(lenSum + 1 + n - 1);
    if (!buf)
        goto error1;
    while (1) {
        nCopied = 0;
        j = 0;
        for (int i = 0; i < n; i++) {
            if (j > 0) {
                buf[nCopied] = colSeparator;
                nCopied++;
            }
            lastReadSize = parseRow(buf + nCopied, strs[i], strs + i, delim);
            if (!lastReadSize)
                continue;
            nCopied += lastReadSize;
            j++;
        }
        if (!j)
            break;
        buf[nCopied] = rowSeparator;
        nCopied++;
        fwrite(buf, 1, nCopied, stdout);
    }
    free(buf);
    return 0;
error1:;
    printErrorMsg("%s", "malloc failed.");
    return 1;
}

int
main(int argc, char **argv)
{
    int i;
    char rowSeparator;
    char colSeparator;
    char delim;
    char *args;
    ArgsLoopState argParseState;

    if (argc <= 1) {
        fputs(usage, stderr);
        exit(1);
    }
    /* set defaults */
    rowSeparator = '\n';
    colSeparator = ' ';
    delim = ' ';
    /* parse arguments */
    argParseState = START;
    for (i = 1; i < argc; i++) {
        args = argv[i];
        switch (argParseState) {
            case START:
                if (args[0] != '-') {
                    /* no more option arguments */
                    goto noMoreArgs;
                } else {
                    switch(args[1]) {
                        case 'r':
                            /* set row separator  */
                            argParseState = ROW_SEPARATOR;
                            break;
                        case 'c':
                            /* set column separator  */
                            argParseState = COL_SEPARATOR;
                            break;
                        case 'd':
                            /* set delimiter  */
                            argParseState = DELIMITER;
                            break;
                        case 'h':
                            /* print usage */
                            puts(usage);
                            exit(0);
                            break;
                        case '-':
                            /* end option arguments */
                            if (args[2]) {
                                ARGS_ERROR;
                            } else {
                                i++;
                                goto noMoreArgs;
                            }
                        default:
                            ARGS_ERROR;
                    }
                    if (args[2])
                        ARGS_ERROR;
                }
                break;
            /* get option arguments */
            case ROW_SEPARATOR:
                TRY_GET_CHAR(&rowSeparator, args);
                break;
            case COL_SEPARATOR:
                TRY_GET_CHAR(&colSeparator, args);
                break;
            case DELIMITER:
                TRY_GET_CHAR(&delim, args);
                if (!delim) {
                    printErrorMsg("The delimiter cannot be NULL!\n", NULL);
                    exit(1);
                }
                break;
        }
        /* 
         * options occure at even indicies.
         * option arguments occure at odd indicies.
         * use these facts to reset the parser state to START when needed
         */
        if (!(i & 1) && argParseState != START)
            argParseState = START;
    }
noMoreArgs:;
    if (i == argc)
        printErrorMsg("Only options were submitted!\n", NULL);
    return printCol(argc - i, argv + i, rowSeparator, colSeparator, delim);
}

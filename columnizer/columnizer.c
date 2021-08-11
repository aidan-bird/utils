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

int isStrAChar(const char *str);
int isStrAEscChar(const char *str);
int parseEscaped(const char *esc);
void printErrorMsg(const char *fmt, ...);
size_t sumStrLen(size_t n, const char **strs);
size_t parseRow(char *restrict buf, char *col, char **nextcol, char delim);
int printCol(size_t n, char **strs, char rowSeperator, char colSeperator,
    char delim);

static const char *usage = 
"USAGE 1: columnizer [OPTIONS]... [STRINGS]...\n"
"USAGE 2: columnizer -- [STRINGS]...\n"
"Formats strings into columns.\n"
"\n"
"OPTIONS\n"
"  -h               Prints this help message.\n"
"  --               Indicates that there are no more option arguments after --.\n"
"  -d [CHARACTER]   Sets the input delimiter character. Space is default.\n"
"  -r [CHARACTER]   Sets the output row separator character. Newline is default.\n"
"  -c [CHARACTER]   Sets the output column separator character. Space is default.\n"
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

int
isStrAEscChar(const char *str)
{
    return str && str[0] == '\\' && str[1] && !str[2];
}

int
tryParseChar(const char *str, char *out)
{
    if (isStrAEscChar(str)) {
        *out = parseEscaped(str + 1);
        return 0;
    } else if (isStrAChar(str)) {
        *out = *str;
        return 0;
    }
    return 1;
}

int
parseEscaped(const char *esc)
{
    if (!esc)
        return 0;
    if (isStrAChar(esc)) {
        switch (*esc) {
            case 'a':
                return '\a';
            case 'b':
                return '\b';
            case 'e':
                return '\e';
            case 'f':
                return '\f';
            case 'n':
                return '\n';
            case 'r':
                return '\r';
            case 't':
                return '\t';
            case 'v':
                return '\v';
            case '\\':
                return '\\';
            case '\'':
                return '\'';
            case '"':
                return '"';
            case '?':
                return '?';
            default:
                return *esc;
        }
    }
    return 0;
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
    /* XXX change this to LF */
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
printCol(size_t n, char **strs, char rowSeperator, char colSeperator, char delim)
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
                buf[nCopied] = colSeperator;
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
        buf[nCopied] = rowSeperator;
        nCopied++;
        fwrite(buf, 1, nCopied, stdout);
    }
    free(buf);
    return 0;
// error2:;
//     free(buf);
error1:;
    return -1;
}

int
main(int argc, char **argv)
{
    int i;
    char rowSeperator;
    char colSeperator;
    char delim;
    int argParseState;
    char *args;

    /* parse args */
    if (argc <= 1) {
        fputs(usage, stderr);
        exit(1);
    }
    rowSeperator = '\n';
    colSeperator = ' ';
    delim = ' ';
    argParseState = 0;
    for (i = 1; i < argc; i++) {
        args = argv[i];
        switch (argParseState) {
            case 0:
                if (args[0] != '-') {
                    goto noMoreArgs;
                } else {
                    switch(args[1]) {
                        case 'r':
                            argParseState = 1;
                            break;
                        case 'c':
                            argParseState = 2;
                            break;
                        case 'd':
                            argParseState = 3;
                            break;
                        case 'h':
                            puts(usage);
                            exit(0);
                            break;
                        case '-':
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
            case 1:
                if (tryParseChar(args, &rowSeperator))
                    EXPECTED_CHAR_ERROR;
                break;
            case 2:
                if (tryParseChar(args, &colSeperator))
                    EXPECTED_CHAR_ERROR;
                break;
            case 3:
                if (tryParseChar(args, &delim))
                    EXPECTED_CHAR_ERROR;
                break;
        }
        if (!(i & 1) && argParseState != 0)
            argParseState = 0;
    }
noMoreArgs:;
    if (i == argc)
        printErrorMsg("Only options were submitted!\n", NULL);
    printCol(argc - i, argv + i, rowSeperator, colSeperator, delim);
    return 0;
}

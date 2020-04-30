#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

static void
usage(char *name)
{
    fprintf(
        stderr,
        "usage : %s [option] <parameter>\n\n"
        "options : \n"
        "    add <string>     create a new entry for <string> in todo list\n"
        "    del <number>     delete entry corresponding to <number> from todo list\n\n"
        "when not provided with any option, the todo list is printed to stdout\n",
        basename(name)
    );

    exit(EXIT_FAILURE);
}

static unsigned
get_num(const char *str)
{
    errno = 0;
    char *ptr;

    long num = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0 || num < 0)
        errx(EXIT_FAILURE, "'%s' isn't a valid positive integer", str);

    return num;
}

static FILE *
open_file(const char *path, const char *mode)
{
    FILE *file = fopen(path, mode);

    if (file == NULL)
        errx(EXIT_FAILURE, "failed to open '%s'", path);

    return file;
}

static void
close_file(const char *path, FILE *file)
{
    if (fclose(file) == EOF)
        errx(EXIT_FAILURE, "failed to close '%s'", path);
}

static inline void *
allocate(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        errx(EXIT_FAILURE, "failed to allocate memory");

    return ptr;
}

static void
print(const char *path)
{
    FILE *file = open_file(path, "r");

    /* add colors if stdout is a terminal */
    const char *format = isatty(fileno(stdout)) == 1
        ? "\033[32m%-*u\033[m%s"
        : "%-*u%s";

    unsigned cnt = 0;
    char input[LINE_MAX];

    while (fgets(input, LINE_MAX, file) != NULL)
        printf(format, PADDING, cnt++, input);

    close_file(path, file);
}

static void
append(const char *path, const char *str)
{
    FILE *file = open_file(path, "a");

    if (fprintf(file, "%s\n", str) < 0)
        errx(EXIT_FAILURE, "failed to write to '%s'", path);

    close_file(path, file);
}

static void
delete(const char *path, const char *str)
{
    const unsigned num = get_num(str);

    FILE *file = open_file(path, "r");

    unsigned tmp = 1;
    unsigned cnt = 0;
    char input[LINE_MAX];

    char **content = allocate(tmp * sizeof *content);

    for (;;) {
        if (fgets(input, LINE_MAX, file) == NULL)
            break;

        content[cnt] = allocate(LINE_MAX * sizeof *content[cnt]);

        strncpy(content[cnt], input, LINE_MAX);
        
        /* allocate more memory if needed */
        if (++cnt == tmp)
            if ((content = realloc(content, (tmp *= 2) * sizeof *content)) == NULL)
                errx(EXIT_FAILURE, "failed to allocate memory");
    }

    close_file(path, file);
    file = open_file(path, "w");

    for (unsigned i = 0; i < cnt; ++i) {
        if (i != num)
            if (fprintf(file, "%s", content[i]) < 0)
                errx(EXIT_FAILURE, "failed to write to '%s'", path);

        free(content[i]);
    }

    free(content);
    close_file(path, file);
}

int
main(int argc, char **argv)
{
    /* obtain path to todo file */
    char path[256] = {0};

    if (snprintf(path, sizeof path, "%s/.local/share/todo", getenv("HOME")) < 0)
        errx(EXIT_FAILURE, "failed to create path to todo file");

    /* parse options */
    switch (argc) {
        case 1:
            print(path);

            break;
        case 3:
            if      (strncmp(argv[1], "add", 4) == 0) append(path, argv[2]);
            else if (strncmp(argv[1], "del", 4) == 0) delete(path, argv[2]);
            else
                usage(argv[0]);

            break;
        default :
            usage(argv[0]);
    }

    return EXIT_SUCCESS;
}

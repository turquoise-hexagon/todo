#include <err.h>
#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "config.h"

static inline void *
allocate(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        errx(EXIT_FAILURE, "failed to allocate memory");

    return ptr;
}

static void
usage(char *name)
{
    fprintf(stderr, "usage: %s [add|del] <string|number>\n", basename(name));

    exit(EXIT_FAILURE);
}

static unsigned
strtou(char *str)
{
    errno = 0;
    char *ptr;

    long tmp = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0 || tmp < 0)
        errx(EXIT_FAILURE, "'%s' isn't a valid positive integer", str);

    return tmp;
}

static FILE *
file_open(const char *name, const char *mode)
{
    FILE *tmp = fopen(name, mode);

    if (tmp == NULL)
        errx(EXIT_FAILURE, "failed to open file '%s'", name);

    return tmp;
}

static void
file_close(FILE *file, const char *name)
{
    if (fclose(file) == EOF)
        errx(EXIT_FAILURE, "failed to close '%s'", name);
}

int
main(int argc, char **argv)
{
    char todo[256] = {0};

    snprintf(todo, sizeof todo, "%s/.local/share/%s", getenv("HOME"), basename(argv[0]));

    switch (argc) {
        case 1:;
            /* print todo-list to stdout */
            FILE *file = file_open(todo, "r");

            const char *format = isatty(fileno(stdout)) == 1
                ? "\033[32m%-*d\033[m%s"
                : "%-*d%s";

            unsigned cnt = 0;
            char input[LINE_MAX];

            while (fgets(input, LINE_MAX, file) != NULL)
                printf(format, PADDING, cnt++, input);

            file_close(file, todo);

            break;
        case 3:
            if (strncmp(argv[1], "add", 4) == 0) {
                /* add new entry to todo-list */
                FILE *file = file_open(todo, "a");

                fprintf(file, "%s\n", argv[2]);

                file_close(file, todo);
            }
            else if (strncmp(argv[1], "del", 4) == 0) {
                /* remove one entry from todo-list */
                const unsigned num = strtou(argv[2]);

                FILE *file = file_open(todo, "r");

                unsigned tmp = 1;
                unsigned cnt = 0;
                char cur[LINE_MAX];

                char **input = allocate(tmp * sizeof *input);

                for (;;) {
                    if (fgets(cur, LINE_MAX, file) == NULL)
                        break;

                    input[cnt] = allocate(LINE_MAX * sizeof *input[cnt]);

                    strncpy(input[cnt], cur, LINE_MAX);

                    if (++cnt == tmp)
                        if ((input = realloc(input, (tmp *= 2) * sizeof *input)) == NULL)
                            errx(EXIT_FAILURE, "failed to allocate memory");
                }

                file_close(file, todo);

                file = file_open(todo, "w");

                for (unsigned i = 0; i < cnt; ++i) {
                    if (i != num)
                        fprintf(file, "%s", input[i]);

                    free(input[i]);
                }

                free(input);
                file_close(file, todo);
            }
            else
                usage(argv[0]);

            break;
        default:
            usage(argv[0]);
    }

    return EXIT_SUCCESS;
}

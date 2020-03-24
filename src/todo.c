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
allocate(unsigned size)
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
strtous(char *str)
{
    errno = 0;
    char *ptr;

    long tmp = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0 || tmp < 0)
        errx(EXIT_FAILURE, "'%s' isn't a valid positive integer", str);

    return (unsigned)tmp;
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
file_close(FILE *file)
{
    if (fclose(file) == EOF)
        errx(EXIT_FAILURE, "failed to close file");
}

int
main(int argc, char **argv)
{
    char todo[256];

    snprintf(todo, sizeof todo, "%s/.local/share/%s", getenv("HOME"), basename(argv[0]));

    switch (argc) {
        case 1 :;
            /* print todo-list to stdout */
            FILE *file = file_open(todo, "r");

            const char *format = isatty(fileno(stdout)) == 1
                ? "\033[32m%-*d\033[m%s"
                : "%-*d%s";

            unsigned cnt = 0;
            char input[LINE_MAX];

            while (fgets(input, LINE_MAX, file) != NULL)
                printf(format, PADDING, cnt++, input);

            file_close(file);

            break;
        case 3 :
            if (strncmp(argv[1], "add", 4) == 0) {
                /* add new entry to todo-list */
                FILE *file = file_open(todo, "a");

                fprintf(file, "%s\n", argv[2]);

                file_close(file);
            }
            else if (strncmp(argv[1], "del", 4) == 0) {
                /* remove one entry from todo-list */
                const unsigned num = strtous(argv[2]);

                FILE *file = file_open(todo, "r");

                unsigned tmp = 1;
                unsigned cnt = 0;
                char cur[LINE_MAX];

                char **input = allocate(tmp * sizeof *input);

                for (;;) {
                    if (fgets(cur, LINE_MAX, file) == NULL)
                        break;

                    input[cnt] = allocate(LINE_MAX * sizeof *input[cnt]);

                    strncpy(input[cnt++], cur, LINE_MAX);

                    if (cnt == tmp) {
                        tmp *= 2;

                        input = realloc(input, tmp * sizeof *input);

                        if (input == NULL)
                            errx(1, "failed to allocate memory");
                    }
                }

                file_close(file);

                file = file_open(todo, "w");

                for (unsigned i = 0; i < cnt; ++i) {
                    if (i != num)
                        fprintf(file, "%s", input[i]);

                    free(input[i]);
                }

                free(input);
                file_close(file);
            }
            else
                usage(argv[0]);

            break;
        default :
            usage(argv[0]);
    }

    return EXIT_SUCCESS;
}

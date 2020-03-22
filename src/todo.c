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
    fprintf(stderr, "usage : %s [add|del] <string|number>\n", basename(name));

    exit(1);
}

static unsigned
strtous(char *str)
{
    errno = 0;
    char *ptr;

    long tmp = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0 || tmp < 0)
        errx(1, "'%s' isn't a valid positive integer", str);

    return (unsigned)tmp;
}

int
main(int argc, char **argv)
{
    /* get path to todo file */
    char todo[256];

    snprintf(todo, sizeof(todo), "%s/.local/share/%s", getenv("HOME"), basename(argv[0]));

    switch (argc) {
        case 1 :;
            /* print todo entries, with line numbers */
            FILE *file = fopen(todo, "r");

            if (file == NULL)
                errx(1, "failed to open '%s'", todo);

            const char *format = isatty(fileno(stdout)) == 1
                ? "\033[32m%-*d\033[m%s"
                : "%-*d%s";

            unsigned cnt = 0;
            char input[LINE_MAX];

            while (fgets(input, LINE_MAX, file) != NULL)
                printf(format, PADDING, cnt++, input);

            fclose(file);

            break;
        case 3 :
            /* add a new entry to the todo file */
            if (strncmp(argv[1], "add", 4) == 0) {
                FILE *file = fopen(todo, "a");

                if (file == NULL)
                    errx(1, "failed to open '%s'", todo);

                fprintf(file, "%s\n", argv[2]);

                fclose(file);
            }
            /* delete one entry from the todo file */
            else if (strncmp(argv[1], "del", 4) == 0) {
                const unsigned num = strtous(argv[2]);

                FILE *file = fopen(todo, "r");

                if (file == NULL)
                    errx(1, "failed to open '%s'", todo);

                unsigned cnt = 0;
                char **input = malloc((cnt + 1) * sizeof *input);

                if (input == NULL)
                    errx(1, "program failed to allocate memory");

                for (;;) {
                    input[cnt] = malloc(LINE_MAX * sizeof *input[cnt]);

                    if (input[cnt] == NULL)
                        errx(1, "program failed to allocate memory");

                    if (fgets(input[cnt++], LINE_MAX, file) == NULL)
                        break;

                    input = realloc(input, (cnt + 1) * sizeof *input);

                    if (input == NULL)
                        errx(1, "program failed to allocate memory");
                }

                fclose(file);

                file = fopen(todo, "w");

                for (unsigned i = 0; i < cnt; ++i)
                    if (i != num)
                        fprintf(file, "%s", input[i]);

                fclose(file);

                for (unsigned i = 0; i < cnt; ++i)
                    free(input[i]);

                free(input);
            }
            else
                usage(argv[0]);

            break;
        default :
            usage(argv[0]);
    }

    return 0;
}

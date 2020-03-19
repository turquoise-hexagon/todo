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
                errno = 0;
                char *ptr;

                const long num = strtol(argv[2], &ptr, 10);

                if (errno != 0 || *ptr != 0 || num < 0)
                    errx(1, "'%s' isn't a valid positive integer", argv[2]);

                FILE *file = fopen(todo, "r");

                if (file == NULL)
                    errx(1, "failed to open '%s'", todo);

                char tmp;
                unsigned cnt = 0;

                while ((tmp = fgetc(file)) != EOF)
                    if (tmp == '\n')
                        ++cnt;

                rewind(file);

                char **input = malloc(cnt * sizeof *input);

                if (input == NULL)
                    errx(1, "program failed to allocate memory");

                for (unsigned i = 0; i < cnt; ++i) {
                    input[i] = malloc(LINE_MAX * sizeof *input[i]);

                    if (input[i] == NULL)
                        errx(1, "program failed to allocate memory");
                }

                cnt = 0;

                while (fgets(input[cnt], LINE_MAX, file) != NULL)
                    ++cnt;

                fclose(file);

                file = fopen(todo, "w");

                if (file == NULL)
                    errx(1, "failed to open '%s'", todo);

                for (unsigned i = 0; i < cnt; ++i)
                    if (i != num)
                        fprintf(file, input[i]);

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

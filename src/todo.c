#include <errno.h>
#include <libgen.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

static const unsigned PADDING = 5;
static const char *CACHE_PATH = ".local/share/todo";

static noreturn void
usage(char *name)
{
    fprintf(
        stderr,
        "usage : %s [-a <string>] [-d <number>]\n"
        "\n"
        "options :\n"
        "    -a <string>     add <string> to todo-list\n"
        "    -d <number>     delete <number>th entry from todo-list\n",
        basename(name));

    exit(1);
}

static inline void *
allocate(size_t size)
{
    void *ptr;

    if (! (ptr = malloc(size))) {
        fprintf(stderr, "error : failed to allocate memory\n");

        exit(1);
    }

    return ptr;
}

static inline void *
reallocate(void *old, size_t size)
{
    void *new;

    if (! (new = realloc(old, size))) {
        fprintf(stderr, "error : failed to reallocate memory\n");

        exit(1);
    }

    return new;
}

static size_t
convert_to_number(const char *str)
{
    errno = 0;
    long num;

    {
        char *ptr;

        if ((num = strtol(str, &ptr, 10)) < 0 || errno != 0 || *ptr != 0) {
            fprintf(stderr, "error : '%s' isn't a valid index\n", str);

            exit(1);
        }
    }

    return (size_t)num;
}

static FILE *
open_file(const char *path, const char *mode)
{
    FILE *file;

    if (! (file = fopen(path, mode))) {
        fprintf(stderr, "error : failed to open '%s'\n", path);

        exit(1);
    }

    return file;
}

static void
close_file(const char *path, FILE *file)
{
    if (fclose(file) != 0) {
        fprintf(stderr, "error : failed to close '%s'\n", path);

        exit(1);
    }
}

static char *
copy_input(const char *str)
{
    char *cpy;

    {
        size_t len;

        len = strnlen(str, LINE_MAX);
        cpy = allocate(len * sizeof(*cpy));
        strncpy(cpy, str, len);

        /* fix string */
        cpy[len - 1] = 0;
    }

    return cpy;
}

static char **
load_content(FILE *file, size_t *size)
{
    char **content;

    {
        size_t allocated = 2;
        size_t assigned  = 0;

        content = allocate(allocated * sizeof(*content));

        char input[LINE_MAX] = {0};

        while (fgets(input, LINE_MAX, file)) {
            content[assigned] = copy_input(input);

            if (++assigned == allocated)
                content = reallocate(content,
                    (allocated = allocated * 3 / 2) * sizeof(*content));
        }

        *size = assigned;
    }

    return content;
}

static void
cleanup_content(char **content, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        free(content[i]);

    free(content);
}

static void
todo_print(const char *path)
{
    FILE *file;

    file = open_file(path, "r");

    {
        char *format;

        /*
         * use pretty formatting when the output
         * terminal is a tty
         */
        format = isatty(fileno(stdout)) == 1
            ? "\033[32m%-*u\033[m%s"
            : "%-*u%s";

        {
            size_t cnt = 0;
            char input[LINE_MAX] = {0};

            while (fgets(input, LINE_MAX, file))
                printf(format, PADDING, cnt++, input);
        }
    }

    close_file(path, file);
}

static void
todo_add(const char *path, const char *str)
{
    FILE *file;

    file = open_file(path, "a");

    if (fprintf(file, "%s\n", str) < 0) {
        fprintf(stderr, "error : failed to write to '%s'\n", path);

        exit(1);
    }

    close_file(path, file);
}

static void
todo_delete(const char *path, size_t index)
{
    FILE *file;

    file = open_file(path, "r");

    size_t size;
    char **content;

    content = load_content(file, &size);

    close_file(path, file);

    file = open_file(path, "w");

    for (size_t i = 0; i < size; ++i)
        if (i != index)
            if (fprintf(file, "%s\n", content[i]) < 0) {
                fprintf(stderr, "error : failed to write to '%s'\n", path);

                exit(1);
            }

    close_file(path, file);
    cleanup_content(content, size);
}

int
main(int argc, char **argv)
{
    char todo_path[PATH_MAX] = {0};

    {
        char *home;

        if (! (home = getenv("HOME"))) {
            fprintf(stderr, "error : failed to get env variable '$HOME'\n");

            exit(1);
        }

        if (snprintf(todo_path, PATH_MAX, "%s/%s", home, CACHE_PATH) < 0) {
            fprintf(stderr, "error : failed to build path to cache file\n");

            exit(1);
        }
    }

    for (int arg; (arg = getopt(argc, argv, ":a:d:")) != -1;)
        switch (arg) {
            case 'a':
                todo_add(todo_path, optarg);

                break;
            case 'd':
                todo_delete(todo_path, convert_to_number(optarg));

                break;
            default :
                usage(argv[0]);
        }

    todo_print(todo_path);

    if (optind < argc)
        usage(argv[0]);

    return 0;
}

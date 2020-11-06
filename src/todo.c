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

static char todo_path[PATH_MAX] = {0};

#define ERROR(code, ...) {        \
    fprintf(stderr, __VA_ARGS__); \
                                  \
    exit(code);                   \
}

static noreturn void
usage(const char *name)
{
    ERROR(
        1,
        "usage : %s [-a <string>] [-d <number>] [-e <number> <string>]\n"
        "\n"
        "options :\n"
        "    -a <string>             add <string> to todo-list\n"
        "    -d <number>             delete <number>th entry from todo-list\n"
        "    -e <number> <string>    replace <number>th entry with <string>\n",
        name)
}

static inline void *
allocate(size_t size)
{
    void *ptr;

    if (! (ptr = malloc(size)))
        ERROR(1, "error : failed to allocate %lu bytes of memory\n", size)

    return ptr;
}

static inline void *
reallocate(void *old, size_t size)
{
    void *new;

    if (! (new = realloc(old, size)))
        ERROR(1, "error : failed to reallocate %lu bytes of memory\n", size)

    return new;
}

static size_t
convert_to_number(const char *str)
{
    errno = 0;
    long res;

    {
        char *ptr;

        if ((res = strtol(str, &ptr, 10)) < 0 || errno != 0 || *ptr != 0)
            ERROR(1, "error : '%s' isn't a valid index\n", str)
    }

    return (size_t)res;
}

static FILE *
open_todo(const char *mode)
{
    FILE *file;

    if (! (file = fopen(todo_path, mode)))
        ERROR(1, "error : failed to open '%s'\n", todo_path)

    return file;
}

static void
close_todo(FILE *file)
{
    if (fclose(file) != 0)
        ERROR(1, "error : failed to close '%s'\n", todo_path)
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
load_todo(size_t *size)
{
    char **content;

    {
        FILE *file;

        file = open_todo("r");

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
        close_todo(file);
    }

    return content;
}

static void
cleanup_todo(char **content, size_t size)
{
    for (size_t i = 0; i < size; ++i)
        free(content[i]);

    free(content);
}

static void
print_todo(void)
{
    FILE *file;

    file = open_todo("r");
    
    {
        char *format;

        /*
         * use pretty formatting when the output
         * terminal is a tty
         */
        format = isatty(fileno(stdout)) == 1
            ? "\033[32m%-*u\033[m%s"
            : "%-*u%s";

        size_t cnt = 0;
        char input[LINE_MAX] = {0};

        while (fgets(input, LINE_MAX, file))
            printf(format, PADDING, cnt++, input);
    }

    close_todo(file);
}

static void
add_todo(const char *str)
{
    FILE *file;

    file = open_todo("a");

    if (fprintf(file, "%s\n", str) < 0)
        ERROR(1, "error : failed to write to '%s'\n", todo_path)

    close_todo(file);
}

static void
delete_todo(size_t index)
{
    FILE *file;

    file = open_todo("r");

    size_t size;
    char **content;

    content = load_todo(&size);

    close_todo(file);

    file = open_todo("w");

    for (size_t i = 0; i < size; ++i)
        if (i != index)
            if (fprintf(file, "%s\n", content[i]) < 0)
                ERROR(1, "error : failed to write to '%s'\n", todo_path)

    close_todo(file);
    cleanup_todo(content, size);
}

static void
edit_todo(size_t index, const char *str)
{
    FILE *file;

    file = open_todo("r");

    size_t size;
    char **content;

    content = load_todo(&size);

    close_todo(file);

    file = open_todo("w");

    for (size_t i = 0; i < size; ++i)
        if (fprintf(file, "%s\n", i == index ? str : content[i]) < 0)
            ERROR(1, "error : failed to write to '%s'\n", todo_path)

    close_todo(file);
    cleanup_todo(content, size);
}

int
main(int argc, char **argv)
{
    const char *name = basename(argv[0]);

    {
        char *home;

        if (! (home = getenv("HOME")))
            ERROR(1, "error : failed to get env variable '$HOME'\n")

        if (snprintf(todo_path, PATH_MAX, "%s/%s", home, CACHE_PATH) < 0)
            ERROR(1, "error : failed to build path to todo file\n")
    }

    for (int arg; (arg = getopt(argc, argv, ":a:d:e")) != -1;)
        switch (arg) {
            case 'a':
                add_todo(optarg);

                break;
            case 'd':
                delete_todo(convert_to_number(optarg));

                break;
            case 'e':
                if (optind < argc - 2)
                    usage(name);

                edit_todo(convert_to_number(argv[optind]), argv[optind + 1]);

                optind += 2;

                break;
            default :
                usage(name);
        }

    print_todo();

    if (optind < argc)
        usage(name);

    return 0;
}

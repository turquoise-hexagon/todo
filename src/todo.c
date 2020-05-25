#include <err.h>
#include <limits.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

#include "config.h"

static void
print_usage(char *program_name)
{
    fprintf(
        stderr,
        "usage : %s [option] <parameter>\n\n"
        "options : \n"
        "    add <string>     create a new entry for <string> in todo-list\n"
        "    del <number>     delete entry corresponding to <number> from todo-list\n\n"
        "when not provided with any option, the todo-list is printed to stdout\n",
        basename(program_name)
    );

    exit(EXIT_FAILURE);
}

/*
 * helper functions
 */
static inline void *
allocate(size_t size)
{
    void *ptr = malloc(size);

    if (ptr == NULL)
        errx(EXIT_FAILURE, "failed to allocate memory");

    return ptr;
}

static unsigned
convert_to_number(const char *str)
{
    errno = 0;
    char *ptr;

    long number = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0 || number < 0)
        errx(EXIT_FAILURE, "'%s' isn't a valid positive integer", str);

    return number;
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

/*
 * main functions
 */
static void
todo_print(const char *path)
{
    FILE *file = open_file(path, "r");

    /* add colors if stdout is a terminal */
    const char *format = isatty(fileno(stdout)) == 1
        ? "\033[32m%-*u\033[m%s"
        : "%-*u%s";

    unsigned cnt = 0;
    char line[LINE_MAX] = {0};

    while (fgets(line, LINE_MAX, file) != NULL)
        printf(format, PADDING, cnt++, line);

    close_file(path, file);
}

static void
todo_append(const char *path, const char *str)
{
    FILE *file = open_file(path, "a");

    if (fprintf(file, "%s\n", str) < 0)
        errx(EXIT_FAILURE, "failed to write to '%s'", path);

    close_file(path, file);
}

static void
todo_delete(const char *path, const char *str)
{
    const unsigned number = convert_to_number(str);

    FILE *file = open_file(path, "r");

    char line[LINE_MAX] = {0};

    size_t number_lines = 0;
    size_t allocated_lines = 1;

    char **file_content = allocate(allocated_lines * sizeof(*file_content));

    /* load file in memory */
    for (;;) {
        if (fgets(line, LINE_MAX, file) == NULL)
            break;

        file_content[number_lines] = allocate(LINE_MAX \
            * sizeof(*file_content[number_lines]));

        strncpy(file_content[number_lines], line, LINE_MAX);

        if (++number_lines == allocated_lines)
            if ((file_content = realloc(file_content, \
                    (allocated_lines *= 2) * sizeof(*file_content))) == NULL)
                errx(EXIT_FAILURE, "failed to allocate memory");
    }

    close_file(path, file);
    file = open_file(path, "w");

    for (size_t i = 0; i < number_lines; ++i) {
        if (i != number)
            if (fprintf(file, "%s", file_content[i]) < 0)
                errx(EXIT_FAILURE, "failed to write '%s'", path);

        free(file_content[i]);
    }

    free(file_content);
    close_file(path, file);
}

int
main(int argc, char **argv)
{
    char todo_path[PATH_MAX] = {0};

    if (snprintf(todo_path, sizeof(todo_path), \
            "%s/.local/share/todo", getenv("HOME")) < 0)
        errx(EXIT_FAILURE, "failed to create path to todo file");

    /* argument parsing */
    switch (argc) {
        case 1:
            todo_print(todo_path);

            break;
        case 3:
            if      (strncmp(argv[1], "add", 4) == 0) todo_append(todo_path, argv[2]);
            else if (strncmp(argv[1], "del", 4) == 0) todo_delete(todo_path, argv[2]);
            else
                print_usage(argv[0]);

            break;
        default:
            print_usage(argv[0]);
    }

    return EXIT_SUCCESS;
}

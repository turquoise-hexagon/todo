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
        "usage : %s [-a <string>] [-d <number>]\n\n"
        "options :\n"
        "    -a <string>    create a new entry for <string> in todo-list\n"
        "    -d <number>    delete entry corresponding to <number> from todo-list\n",
        basename(name));

    exit(1);
}

/*
 * helper functions
 */
static inline void *
allocate(size_t size)
{
    void *ptr;

    if ((ptr = malloc(size)) == NULL)
        errx(1, "failed to allocate memory");

    return ptr;
}

static unsigned
convert_to_number(const char *str)
{
    errno = 0;

    char *ptr;
    long number;

    number = strtol(str, &ptr, 10);

    if (errno != 0 || *ptr != 0 || number < 0)
        errx(1, "'%s' isn't a valid positive integer", str);

    return (unsigned)number;
}

static FILE *
open_file(const char *path, const char *mode)
{
    FILE *file;

    if ((file = fopen(path, mode)) == NULL)
        errx(1, "failed to open '%s'", path);

    return file;
}

static void
close_file(const char *path, FILE *file)
{
    if (fclose(file) == EOF)
        errx(1, "failed to close '%s'", path);
}

static char **
load_file(FILE *file, size_t *file_length)
{
    size_t number_lines = 0;
    size_t allocated_lines = 1;
    char line[LINE_MAX] = {0};

    char **file_content;

    file_content = allocate(allocated_lines * sizeof(*file_content));

    /* load file in memory */
    for (;;) {
        if (fgets(line, LINE_MAX, file) == NULL)
            break;

        file_content[number_lines] = allocate(LINE_MAX *
            sizeof(*file_content[number_lines]));

        strncpy(file_content[number_lines], line, LINE_MAX);

        if (++number_lines == allocated_lines)
            if ((file_content = realloc(file_content,
                    (allocated_lines *= 2) * sizeof(*file_content))) == NULL)
                errx(1, "failed to allocate memory");
    }

    *file_length = number_lines;
    return file_content;
}

/*
 * main functions
 */
static void
todo_print(const char *path)
{
    FILE *file;
    char *format;

    file = open_file(path, "r");

    /* add colors if stdout is a terminal */
    format = isatty(fileno(stdout)) == 1
        ? "\033[32m%-*u\033[m%s"
        : "%-*u%s";

    unsigned count = 0;
    char line[LINE_MAX] = {0};

    while (fgets(line, LINE_MAX, file) != NULL)
        printf(format, PADDING, count++, line);

    close_file(path, file);
}

static void
todo_append(const char *path, const char *str)
{
    FILE *file;

    file = open_file(path, "a");

    if (fprintf(file, "%s\n", str) < 0)
        errx(1, "failed to write to '%s'", path);

    close_file(path, file);
}

static void
todo_delete(const char *path, const char *str)
{
    /* get line to delete */
    unsigned number;

    number = convert_to_number(str);

    /* get file content */
    FILE *file;

    file = open_file(path, "r");

    size_t file_length;
    char **file_content;

    file_content = load_file(file, &file_length);

    close_file(path, file);

    /* overwrite with new content */
    file = open_file(path, "w");

    for (size_t i = 0; i < file_length; ++i) {
        if (i != number)
            if (fprintf(file, "%s", file_content[i]) < 0)
                errx(1, "failed to write to '%s'", path);

        free(file_content[i]);
    }

    free(file_content);
    close_file(path, file);
}

int
main(int argc, char **argv)
{
    /* obtain path to todo file */
    char todo_path[PATH_MAX] = {0};

    if (snprintf(todo_path, sizeof(todo_path),
           "%s/.local/share/todo", getenv("HOME")) < 0)
        errx(1, "failed to obtain path to todo file");

    /* argument parsing */
    for (int arg; (arg = getopt(argc, argv, ":a:d:")) != -1;)
        switch (arg) {
            case 'a': todo_append(todo_path, optarg); break;
            case 'd': todo_delete(todo_path, optarg); break;
            default:
                usage(argv[0]);
        }

    if (optind < argc) /* handle mismatched parameters */
        usage(argv[0]);

    todo_print(todo_path);

    return 0;
}

----
todo
----

a simple todo-list program
==========================

:date: March 2020
:version: 0.0
:manual section: 1
:manual group: General Commands Manual

synopsis
--------
| todo [-a `<string>`] [-d `<number>`] [-e `<number>` `<string>`]
| todo

description
-----------
todo stores, displays and manages a simple plain text, line separated todo-list

it aims at being minimalist, fast and easy to use / script

options
-------
-a `<string>`
    add <string> to todo-list\n"
-d `<number>`
    delete <number>th entry from todo-list\n"
-e `<number>` `<string>`
    replace <number>th entry with <string>\n",

example
-------
::

    $ todo -a 'a'
    0    a
    $ todo -a 'c'
    0    a
    1    c
    $ todo -e 1 'b'
    0    a
    1    b
    $ todo -d 0
    0    b

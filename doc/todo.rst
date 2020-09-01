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
| todo [-a `<string>`] [-d `<number>`]
| todo

description
-----------
todo stores, displays and manages a simple plain text, line separated todo-list

it aims at being minimalist, fast and easy to use / script

options
-------
-a `<string>`
    add <string> to todo-list
-d `<number>`
    delete <number>th entry from todo-list

example
-------
::

    $ todo -a 'hello'
    0    hello
    $ todo -a 'world'
    0    hello
    1    world
    $ todo -d 0
    0    world

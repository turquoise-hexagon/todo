----
todo
----

a simple todo list program
==========================

:date: March 2020
:version: 0.0
:manual section: 1
:manual group: General Commands Manual

synopsis
--------
| todo `[option]` <parameter>
| todo

description
-----------
todo stores, displays and manages a simple plain text, line separated todo list

it aims at being minimalist, fast and easy to use / script

options
-------
``add <string>``
    create a new entry for <string> in todo list
``del <number>``
    delete entry corresponding to <number> from todo list

when not provided with any option, the todo list is printed to stdout

example
-------
::

    $ todo add "hello"
    $ todo add "world"
    $ todo
    0    hello
    1    world
    $ todo del 0
    $ todo
    0    world

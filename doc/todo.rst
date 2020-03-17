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
| todo `[option]` <parameter>
| todo

description
-----------
todo stores, displays and manages a simple plain text, line separated todo-list

it aims at being minimalist, fast and easy to use / script

options
-------
``add <string>``
    add a string to the todo-list
``del <number>``
    delete the nth entry of the todo-list

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

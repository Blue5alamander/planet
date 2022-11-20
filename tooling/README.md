# Game tooling

Various tooling utilities to help with things like game distribution and asset management etc.


## `linux-builder` container

Steam's Linux run time unfortunately doesn't contain the latest compilers. The general process to create a set of files for distribution is:

Firstly, run the `create` script. The script will fetch the clang source code, configure it and build it using Valve's run time and put it into the `linux-builder` container.

The container itself is big, the clang sources are huge and the compilation will take a long time and produce another fairly large container -- this is not to be undertaken by anybody with a slow machine or slow Internet connection.


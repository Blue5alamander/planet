# `linux-builder` container

Steam's Linux run time unfortunately doesn't contain the latest compilers. Using the `create` script in this folder will fix this for you, with a brand new clang compiler to use.

The general process to create a set of files for distribution is:

Firstly, run the `create` script. The script will fetch the clang source code, configure it and build it using Valve's run time and put it into the `linux-builder` container.

The container itself is big, the clang sources are huge and the compilation will take a long time and produce another fairly large container -- this is not to be undertaken by anybody with a slow machine or slow Internet connection.

Once you have the container it can be used to create your distribution files for the game code.

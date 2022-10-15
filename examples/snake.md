# Snake

What if snake was a 1980s text adventure game?

This is the classic snake game, but it's turn based and played on a hex grid where you can only see your immediate surroundings.

A game start:

```
Welcome to snake

Type one of 'ne', 'nw', 'w', 'e', 'se', 'sw' followed by enter to move in that direction
You can also ask to 'draw' the map

  .   f

.   h   .

  .   .

> ne
You ate some food.

  .   .

.   h   o

  .   .
```

And after getting some power up:

```
> ne
Uh oh, you got shorter

                    .

          o   v   .   .   .   o

        .   .   o   .   F   .   .

      o   o   .   o   o   .   o   .

    .   o   o   .   F   .   .   v   o

  o   F   o   o   v   .   .   o   o   o

.   o   o   o   o   h   .   o   o   .   .

  o   o   v   .   s   o   .   o   o   .

    o   o   o   s   o   .   .   o   .

      o   o   o   s   o   o   f   o

        o   .   .   s   .   o   .

          o   o   .   .   .   .

                    .
```

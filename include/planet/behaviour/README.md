# Behaviours

A good introduction to behaviour trees can be found at [Game Developer](https://www.gamedeveloper.com/programming/behavior-trees-for-ai-how-they-work) by the developer of _Project Zomboid_.

* [`planet/behaviour/context.hpp`](./context.hpp) -- The `context` type stores the variable assignments that the behaviours are able to use.
* [`planet/behaviour/decorators.hpp`](./decorators.hpp) -- Contains implementations of the decorators. These are behaviours that can be used to alter the outcome of another behaviour.
* [`planet/behaviour/key.hpp`](./key.hpp) -- Contains the `key` which describes the lookup for a named variable available to call a behaviour with.
* [`planet/behaviour/leaf.hpp`](./leaf.hpp) -- Leaf behaviours have to be implemented within each game the behaviours are used in. The various `create` overloads simplify the binding between coroutines and behaviours.
* [`planet/behaviour/parameter.hpp`](./parameter.hpp) -- Describes a parameter that is available to a behaviour to take as an argument.
* [`planet/behaviour/runner.hpp`](./runner.hpp) -- The `runner` type is an abstract interface that all behaviours conform to.
* [`planet/behaviour/state.hpp`](./state.hpp) -- The `state` enumeration used to describe behaviour outcomes.

There is also the convenience header [`planet/behaviour.hpp`](../behaviour.hpp) which includes everything.


## Usefulness?

It does seem that a the behaviours, as described for use in _Project Zomboid_, do seem to be just a form of more scripting. The tree as described is merely an abstract syntax tree describing how things need to interact. Probably a DSL would work even better and be much easier to work with.

It's likely better to think of this as an execution engine for a DSL and then build a DSL later on.

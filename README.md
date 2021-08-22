# lock-free-object-pool
An efficient thread-safe lock-free object pool. Inspired by CCiA.

## Motivation
Object Pool is a useful util for avoiding memory allocation and class construction in runtime. It allocates, constructs a bunch of objects before service's start. The process get one from pool when needed, reclaim it after used.
However, it becomes more complicated in a concurrent circumanstance. Usually, locks/mutexs will be used to avoid contention raised by multi-threads accessing. In Chap.7 of CCiA, lock-free-stack was illustrated with details of
implementation. It inspires me that a lock-free structure can be used to impliment object pool. When get one object from pool, an instance is out of stack, and when reclaim one, the instance was push into the stack.

## Efficiency
I do not have comparison result to other object-pools. But, in our online service, when I test it with an element-wised-lock object pool, and it turns out the lock-free-object-pool saves 2~3ms on avg and tp99. When a thread trys to
get one with element-wised-lock pool, it has to calculate a random num at first, and trys to lock this element after. If it fails to get that one, then it trys to access next one just like the first one until it reaches max-try-num.
When with lock-free object pool, there is always a thread wins an object, and it saves time when most objects in pool are occupied.

## ToDo
- more useful apis

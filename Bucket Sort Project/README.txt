Explanation of reasons for the differences in each parallel version.

VERSION 1: By using spinlock, program will be limited performance because bnum is updated frequently with the large number of list.

VERSION 2: By using array of locks, we can improve performance. It can reduce the mutual exclusion.

VERSION 3: We can make the program faster by not using shared variables or locks. It can avoid the mutual exclusion time.

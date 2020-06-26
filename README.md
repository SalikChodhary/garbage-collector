# garbage-collector
My attempt at making a simple garbage collector for C. This garbage collector follows the mark and sweep algorithm, and works perfectly! 

The GC works perfectly, given that your C program only utilizes two types, which are defined in the program (int and pair). These limitations are for demonstrational purposes, the program can easily be modified to accept a wide range of types.

For tesing purposes, add tests to main and don't forget to free the VM, at the end of every test!

Credits to: [Bob Nystrom](http://journal.stuffwithstuff.com/2013/12/08/babys-first-garbage-collector/)

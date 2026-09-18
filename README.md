Taken from: https://www.cpp.edu/~carich/programming_contests/200802/quodigious.pdf (this is a dead link with no archive available...)

A quodigious number is a positive decimal number whose digits are each greater
than 1, and which is evenly divisible by both the sum and product of its
digits.

Believe it or not, identifying these numbers are really really time consuming.


History
-------
Back in the summer of 2008 when I was just a wee lad at Cal Poly Pomona, I
participated in a programming contest hosted by the computer science society.
In the contest there was one question which I solved but was never ever truely
satisifed with how long it took. I have spent time since then to try and speed
it up. Every so often I will try to speed it up but never to the degree that I
have since I started this repo.

The history of this repo shows my adventures of porting the solver from C# to
C++14 and all of the missteps in between.

In the original programming contest question, the number of digits was locked
to less than 10 so it would fit in a 32-bit integer (signed or unsigned). I had
decided to keep going and use a 64-bit unsigned integer. This allows digit
counts of up to 19 digits! 


Computation Technique
---------------------

Instead of generating each number and then breaking it apart, the modern versions of this program actually just build the number digit by digit using recursion.  This allows for easy sum and product calculation. It also makes it possible to introduce threading at different layers. 

Previously, we started at the largest digit and walked _down_ to the least significant digit. Since 9/15/2026, I have come up with a _reverse_ direction implementation where we start at the
least significant digit and work our way up towards the most significant digit. This has some interesting benefits including:

- Able to compute all quodigious numbers up to the width specified instead of _only_ at the width specified! This has the benefit of improving performance by not requiring recomputation of values. Instead we just walk into the number computation and check for quodigious-ness as we walk through. We stop once we get to the defined depth
- No need for a lookup table for the factors of computation (we can just use templates and multiple each depth by 10).
- We can construct a _histogram_-like image of the numbers if we print out these values as we walk through the network.
- Patterns in the set of quodigious numbers is far more obvious in the reverse format instead of the forward format. For example, no quodigious numbers have 3, 7, or 9 as their least significant digit. This makes it possible to eliminate up to 3/8th of the compute space and then allows us to go back and test those spaces separately if needed
- Each subset of the execution graph can be thrown onto a separate thread. So LSB's of 2, 3, 4, 5, 6, 7, 8, and 9 are all thrown onto their own thread. We can go one step further and do the next depth in as well if desired.



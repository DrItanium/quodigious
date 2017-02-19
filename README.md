Taken from: https://www.cpp.edu/~carich/programming_contests/200802/quodigious.pdf

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


Tricks used
-----------
My whole objective in starting this repo is to show various tricks that I used
to greatly speed up the process of identifying these numbers. Some of these
tricks include:

Starting at (mul (pow 10 ?width) (+ 2.0 (/ 2.0 9.0))) to eliminate a huge
majority of numbers which will never ever be legal due to the fact they have
the digits 0 and 1 in them.

Memoizing the sums, products, and legality of numbers with digital widths of
3,4,5,6,7. Going above these values requires a fuck ton of ram with very little
benefit (cache thrash anyone?). Even at this point, the program will use
roughly 185 megs of ram! To save space, the sums are a single list where the
products and predicates have to be separate lists for each width.

Std::async is a wonderful drug :D. I use it like crazy and I spawn about 6
threads (plus the main one) during execution of widths of 7 or more. 


I can't find any quodigious numbers above widths of 3 digits that contain the
number 5! I haven't been able to verify this for 14-19 digits (without the
code I have now it would've taken in literal years to compute!) though. So in
the case of 4 digits through 13 digits I do not even bother computing any
numbers whose most significant digit is 5!


In essence all of the code I have written is centered around reducing the
scanning space. If there is a constant time solution to this problem I would
love to know :).

I love using templates to write code for me :D. The problem with using
templates though is the amount of time it takes to compile. In the case of this
program, it takes upwards of 18 seconds to compile and consumes at least a
gigabyte of RAM! The 29k executable balloons to 1.4 megabytes (pre strip) with
all of the templating I use :D. I'm super lazy!


Rant
----
Fuck C#, it's a slow language for doing things like numeric computation (yes,
the JIT does speed it up greatly but the C++14 implementation is just
so much faster :D.

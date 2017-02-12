# This test is a remix of the Fibonacci test with threading added
program threading;

//! Fibonacci (recursive way)
immutable function Fibonacci(n) : typeof n
{
   if n < 2 then
       n
   else
       Fibonacci(n - 1) + Fibonacci(n - 2)
}

function main
{
   // Compute the result of `a` into another thread
   a := & Fibonacci(40);
   // Compute the result of `b` into another thread
   b := & Fibonacci(60);
   // Wait for the result of a and b, then compute a+b
   c := a + b;
   // Print the result
   println("a + b = ", c)
}

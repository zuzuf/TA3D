# This test checks a simple recursive program with no class, and simple math
program fibonacci;

#! \brief Fibonacci (recursive way)
immutable function Fibonacci(n) : typeof n
{
   if n < 2 then
       return n;
   return Fibonacci(n - 1) + Fibonacci(n - 2);
}

#! \brief Fibonacci (recursive way)
immutable function Fibonacci2(n)
{
   if n < 2 then n else Fibonacci(n - 1) + Fibonacci(n - 2)
}


function main : int
{
   print(Fibonacci(10));
   return 0;
}

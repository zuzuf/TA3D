# This test contains function specializations
program Specializations;

#! Default behaviour
function fibonacci(n)
in
	n > 0 else throw;
{
	fibonacci(n - 1) + fibonacci(n - 2)
}

#! Specialization
function fibonacci(n)
when n < 2
{
	n
}


function main
{
	fibonacci(12)
}

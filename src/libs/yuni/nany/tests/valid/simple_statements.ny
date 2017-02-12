# This test checks for several simple statements
program SimpleStatements;

uses Dummy;
uses Dummy.Stupid;
uses Dummy.Stupid.Clever;
uses Dummy.Clever;

function main
{
        err := 1;
        if false then
            return err;
    a: uint16 := 42;
    {
        err := 1;
        if false then
            return if err < 1 then 10 else ++a.beta.foo;
    }
    while a > 0 do
    {
        --a;
        if a = 0 then
            break;
        else
            continue;
    }
    return 0;
}

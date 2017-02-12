-- Lua basic program

function printRandomArray()
	io.write("[ Lua] Dumping the C++ generated sampleArray: {");
	for i = 1, #sampleArray, 1 do
		if i ~= 1 then
			io.write(",");
		end
		io.write(" " ..sampleArray[i]);
	end
	io.write(" }\n");
	return 1;
end

function sampleLuaFunction(arg, arg2)
	print ("[ Lua] Now in sampleLuaFunction(" .. arg .. ", " .. arg2 .. ")");
	return "Return from sampleLuaFunction";
end

function callBackSampleFunctions()
	print ("[ Lua] sampleFunction(4242, 2.4, 'nyu', 0) returned [" .. sampleFunction(4242, 2.4, "nyu", 0) .. "].");
	print ("[ Lua] SampleObject__sampleMethod('Hello world !') returned [" .. SampleObject__sampleMethod("Hello world") .. "].");
end

function printNumber(i)
	io.write("[ Lua] Number: " .. i .. "\n")
end

print "[ Lua] helloworld.lua has been parsed and executed."


YUNITESTCASE_BEGIN(libyuni_core_string_assign_char
	"
	options.description = 'String Assignment (StringBase<char>)'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = 'result.stdout.txt'
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(main_char.cpp common.hxx)
YUNITESTCASE_END()


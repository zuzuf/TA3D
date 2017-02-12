
YUNITESTCASE_BEGIN(libyuni_core_string_constructor_char
	"
	options.description = 'String instanciation in various ways (StringBase<char>)'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = 'result.stdout.txt'
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(main_char.cpp common.hxx)
YUNITESTCASE_END()


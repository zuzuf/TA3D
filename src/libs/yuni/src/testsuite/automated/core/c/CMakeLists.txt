
YUNITESTCASE_BEGIN(libyuni_core_c_header
	"
	options.description = 'Compliant with a C compiler'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(main.c)
YUNITESTCASE_END()



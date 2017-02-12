
YUNITESTCASE_BEGIN(libyuni_core_system_environment_read
	"
	options.description = 'Reading values from the current environment'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(read.cpp)
YUNITESTCASE_END()



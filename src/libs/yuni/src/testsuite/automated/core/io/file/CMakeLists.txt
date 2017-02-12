
YUNITESTCASE_BEGIN(libyuni_core_io_exists
	"
	options.description = 'File or directory existence'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(exists.cpp)
YUNITESTCASE_END()




YUNITESTCASE_BEGIN(libyuni_core_io_file_exists
	"
	options.description = 'File existence'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(file.exists.cpp)
YUNITESTCASE_END()




YUNITESTCASE_BEGIN(libyuni_core_io_directory_exists
	"
	options.description = 'Directory existence'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(directory.exists.cpp)
YUNITESTCASE_END()




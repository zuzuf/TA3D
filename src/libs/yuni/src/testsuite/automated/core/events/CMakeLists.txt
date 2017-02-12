
YUNITESTCASE_BEGIN(libyuni_core_events_simple
	"
	options.description = 'Event'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(01.simple.cpp)
YUNITESTCASE_END()


YUNITESTCASE_BEGIN(libyuni_core_events_method
	"
	options.description = 'Connected to a method'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(02.method.cpp)
YUNITESTCASE_END()


YUNITESTCASE_BEGIN(libyuni_core_events_clear
	"
	options.description = 'Clear'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(03.clear.cpp)
YUNITESTCASE_END()


YUNITESTCASE_BEGIN(libyuni_core_events_remove
	"
	options.description = 'Remove'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(04.remove.cpp)
YUNITESTCASE_END()


YUNITESTCASE_BEGIN(libyuni_core_events_remove_with_inherited_class
	"
	options.description = 'Remove with inherited class'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(05.remove.with.inherited.class.cpp)
YUNITESTCASE_END()



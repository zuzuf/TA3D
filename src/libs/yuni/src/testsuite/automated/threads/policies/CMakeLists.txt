
YUNITESTCASE_BEGIN(libyuni_threads_policies_singlethreaded
	"
	options.description = 'Threading policy: Single threaded'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(main_single.cpp)
	YUNITESTCASE_LINK(${YUNI_STATIC_CORE})
YUNITESTCASE_END()


YUNITESTCASE_BEGIN(libyuni_threads_policies_objectlevellockable
	"
	options.description = 'Threading policy: Object Level Lockable'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(main_objectlevel.cpp)
	YUNITESTCASE_LINK(${YUNI_STATIC_CORE})
YUNITESTCASE_END()


YUNITESTCASE_BEGIN(libyuni_threads_policies_classlevellockable
	"
	options.description = 'Threading policy: Class Level Lockable'
	options.args = ''
	options.timeout = 5 # seconds
	options.compareStdoutWith = nil
	options.compareStderrWith = nil
	options.exitStatus = 0
	")
	YUNITESTCASE_MODULES(core)
	YUNITESTCASE_SOURCE(main_classlevel.cpp)
	YUNITESTCASE_LINK(${YUNI_STATIC_CORE})
YUNITESTCASE_END()





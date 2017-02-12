#ifndef __YUNI_PRIVATE_SCRIPT_DEFINES_H__
# define __YUNI_PRIVATE_SCRIPT_DEFINES_H__

# define YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Class, ...) \
	bool Class::call(Any *retValues, const String& method, ##__VA_ARGS__)

#endif /* !__YUNI_PRIVATE_SCRIPT_DEFINES_H__ */


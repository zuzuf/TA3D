#ifndef __YUNI_SCRIPT_SCRIPT_DEFINES_H__
# define __YUNI_SCRIPT_SCRIPT_DEFINES_H__

/*!
** \file
** Defines macros for method declaration in AScript & co.
** The macros in this file are used to declare functions with
** various argument counts (bind() and call()) in AScript declaration,
** and declarations of all classes inheriting from it.
*/

# define YUNI_SCRIPT_SCRIPT_ANY(name) \
	const Any & name

# define YUNI_SCRIPT_SCRIPT_1_ANY \
	YUNI_SCRIPT_SCRIPT_ANY(arg1)

# define YUNI_SCRIPT_SCRIPT_2_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2)

# define YUNI_SCRIPT_SCRIPT_3_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2), \
	YUNI_SCRIPT_SCRIPT_ANY(arg3)

# define YUNI_SCRIPT_SCRIPT_4_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2), \
	YUNI_SCRIPT_SCRIPT_ANY(arg3), \
	YUNI_SCRIPT_SCRIPT_ANY(arg4)


# define YUNI_SCRIPT_SCRIPT_5_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2), \
	YUNI_SCRIPT_SCRIPT_ANY(arg3), \
	YUNI_SCRIPT_SCRIPT_ANY(arg4), \
	YUNI_SCRIPT_SCRIPT_ANY(arg5)



# define YUNI_SCRIPT_SCRIPT_6_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2), \
	YUNI_SCRIPT_SCRIPT_ANY(arg3), \
	YUNI_SCRIPT_SCRIPT_ANY(arg4), \
	YUNI_SCRIPT_SCRIPT_ANY(arg5), \
	YUNI_SCRIPT_SCRIPT_ANY(arg6)


# define YUNI_SCRIPT_SCRIPT_7_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2), \
	YUNI_SCRIPT_SCRIPT_ANY(arg3), \
	YUNI_SCRIPT_SCRIPT_ANY(arg4), \
	YUNI_SCRIPT_SCRIPT_ANY(arg5), \
	YUNI_SCRIPT_SCRIPT_ANY(arg6), \
	YUNI_SCRIPT_SCRIPT_ANY(arg7)


# define YUNI_SCRIPT_SCRIPT_8_ANYS \
	YUNI_SCRIPT_SCRIPT_ANY(arg1), \
	YUNI_SCRIPT_SCRIPT_ANY(arg2), \
	YUNI_SCRIPT_SCRIPT_ANY(arg3), \
	YUNI_SCRIPT_SCRIPT_ANY(arg4), \
	YUNI_SCRIPT_SCRIPT_ANY(arg5), \
	YUNI_SCRIPT_SCRIPT_ANY(arg6), \
	YUNI_SCRIPT_SCRIPT_ANY(arg7), \
	YUNI_SCRIPT_SCRIPT_ANY(arg8)


# define YUNI_SCRIPT_SCRIPT_DECLARE_CALL_WITH(...) \
	virtual bool call(Any *retValues, const String& method, ## __VA_ARGS__)

#endif /* !__YUNI_SCRIPT_SCRIPT_DEFINES_H__ */


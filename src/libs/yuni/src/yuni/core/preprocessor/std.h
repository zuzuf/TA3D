#ifndef __YUNI_CORE_PREPROCESSOR_STANDARD_H__
# define __YUNI_CORE_PREPROCESSOR_STANDARD_H__

/* !!! "C compatibility" header !!! */


/*! String concatenation */
# define YUNI_JOIN(X,Y)  X ## Y

/*! Convenient define to deal with temporary (or not) unused parameter */
# define YUNI_UNUSED_ARGUMENT(X) (void)(X)


/*! The identity function */
# define YUNI_IDENTITY(...) __VA_ARGS__

/*! An empty value */
# define YUNI_EMPTY

/*! Comma */
# define YUNI_COMMA ,

/*! Semicolon */
# define YUNI_SEMICOLON ;

/*! Dot */
# define YUNI_DOT .

/*! Minus */
# define YUNI_MINUS -



#endif /* __YUNI_CORE_PREPROCESSOR_STANDARD_H__ */

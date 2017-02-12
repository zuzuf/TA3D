#ifndef __YUNI_CORE_PREPROCESSOR_MATH_H__
# define __YUNI_CORE_PREPROCESSOR_MATH_H__



/*!
** \brief Compute x + y
*/
# define YUNI_ADD(x, y)  ((x) + (y))

/*!
** \brief Compute x - y
*/
# define YUNI_SUB(x, y)  ((x) - (y))

/*!
** \brief Compute x % y
*/
# define YUNI_MOD(x, y)  ((x) % (y))

/*!
** \brief Compute x / y
*/
# define YUNI_DIV(x, y)  ((x) / (y))

/*!
** \brief Compute x * y
*/
# define YUNI_MUL(x, y)  ((x) * (y))





/*!
** \brief Logic: x && y
*/
# define YUNI_AND(x, y)  ((x) && (y))

/*!
** \brief Logic: x || y
*/
# define YUNI_OR(x, y)  ((x) || (y))

/*!
** \brief Logic: !(x || y)
*/
# define YUNI_NOR(x, y)  (!((x) || (y)))

/*!
** \brief Logic: not
*/
# define YUNI_NOT(x)  (!((x)))





/*!
** \brief Logic: x & y
*/
# define YUNI_BITAND(x, y)  ((x) & (y))

/*!
** \brief Logic: x | y
*/
# define YUNI_BITOR(x, y)  ((x) | (y))

/*!
** \brief Logic: !(x | y)
*/
# define YUNI_BITNOR(x, y)  (!((x) | (y)))



#endif /* __YUNI_CORE_PREPROCESSOR_MATH_H__ */

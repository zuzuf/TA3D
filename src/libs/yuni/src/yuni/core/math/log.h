#ifndef __YUNI_CORE_MATH_LOG_H__
# define __YUNI_CORE_MATH_LOG_H__

# include "math.h"



//! The constant value for `log(2)`
# define YUNI_LOG_2   0.301029996



namespace Yuni
{
namespace Math
{


	/*!
	** \brief The natural logarithm function
	*/
	template<typename T> inline T Log(T x);

	/*!
	** \brief The base-2 logarithm function
	*/
	template<typename T> inline T Log2(T x);


	/*!
	** \brief Compute log(1 + x)
	**
	** If x is very small, directly computing log(1+x) can be inaccurate and the
	** result may return log(1) = 0. All precision is lost.
	** \code
	** std::cout << Math::Log(1 + 1e-16) << std::endl; // = log(1) = 0
	** \endcode
	**
	** We can avoid this problem by using a Taylor series
	** log(1+x) ≈ x - (x*x)/2
	**
	** \param x Any value > -1.0
	** \return log(1+x). when x < 1e-4, the Taylor series approximation will be used
	*/
	template<typename T> inline T LogOnePlusX(T x);


} // namespace Math
} // namespace Yuni

# include "log.hxx"

#endif // __YUNI_CORE_MATH_LOG_H__

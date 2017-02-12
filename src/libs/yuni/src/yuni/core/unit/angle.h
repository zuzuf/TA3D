#ifndef __YUNI_CORE_UNIT_ANGLE_H__
# define __YUNI_CORE_UNIT_ANGLE_H__

# include "unit.h"
# include "define.h"


namespace Yuni
{
namespace Unit
{

/*!
** \defgroup UnitAngle Angle
** \ingroup Units
*/

/*!
** \brief Angle
** \ingroup UnitAngle
*/
namespace Angle
{
	//! \ingroup UnitAngle
	struct Quantity;


	//! \brief SI (Radia)
	//! \ingroup UnitAngle
	YUNI_UNIT_IMPL(SIBaseUnit, "radian", "rad", int, 1);

	//! \brief Radian
	//! \ingroup UnitAngle
	YUNI_UNIT_IMPL(Radian, "radian", "rad", int, 1);

	//! \brief Degree (of arc)
	//! \ingroup UnitAngle
	YUNI_UNIT_IMPL(Degree, "degree", "°", double, 17.453293e-3);



} // namespace Length
} // namespace Unit
} // namespace Yuni

# include "undef.h"

#endif/// __YUNI_CORE_UNIT_ANGLE_H__

#ifndef __YUNI_CORE_UNIT_LUMINANCE_H__
# define __YUNI_CORE_UNIT_LUMINANCE_H__

# include "unit.h"
# include "define.h"


namespace Yuni
{
namespace Unit
{


/*!
** \defgroup UnitLuminance  Luminance
** \ingroup Units
*/

/*!
** \brief Luminance
** \ingroup UnitLuminance
*/
namespace Luminance
{
	//! \ingroup UnitLuminance
	struct Quantity;

	//! \brief SI (Candela per square metre)
	//! \ingroup UnitLuminance
	YUNI_UNIT_IMPL(SIBaseUnit, "candela", "", int, 1);

	//! \brief Candela (per square metre)
	//! \ingroup UnitLuminance
	YUNI_UNIT_IMPL(Candela, "candela", "", int, 1);
	//! \brief Lambert
	//! \ingroup UnitLuminance
	YUNI_UNIT_IMPL(Lambert, "lambert", "L", double, 3183.09886);



} // namespace Luminance
} // namespace Unit
} // namespace Yuni

# include "undef.h"

#endif // __YUNI_CORE_UNIT_LUMINANCE_H__

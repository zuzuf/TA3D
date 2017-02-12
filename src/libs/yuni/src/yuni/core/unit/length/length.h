#ifndef __YUNI_CORE_UNIT_LENGTH_LENGTH_H__
# define __YUNI_CORE_UNIT_LENGTH_LENGTH_H__

# include "../unit.h"
# include "../define.h"


namespace Yuni
{
namespace Unit
{

/*!
** \defgroup UnitLength  Length
** \ingroup Units
*/


/*!
** \brief Length
** \ingroup Unit
*/
namespace Length
{
	//! \ingroup UnitLength
	struct Quantity;


	//! \brief Metre (Distance light travels in 1/299 792 458 of a second in vacuum)
	//! \ingroup UnitLength
	YUNI_UNIT_IMPL(SIBaseUnit, "metre", "m", int,1);


} // namespace Length
} // namespace Unit
} // namespace Yuni

# include "../undef.h"

#endif/// __YUNI_CORE_UNIT_LENGTH_LENGTH_H__

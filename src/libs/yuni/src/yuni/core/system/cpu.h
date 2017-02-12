#ifndef __YUNI_CORE_SYSTEM_CPU_H__
# define __YUNI_CORE_SYSTEM_CPU_H__

# include "../../yuni.h"


namespace Yuni
{
namespace System
{
namespace CPU
{

	/*!
	** \brief Get the number of system CPU
	**
	** \return The number of CPU (logic or not), 1 when the value is unknown
	*/
	unsigned int Count();


} // namespace CPU
} // namespace System
} // namespace Yuni

#endif // __YUNI_CORE_SYSTEM_CPU_H__

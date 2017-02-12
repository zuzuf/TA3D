#ifndef __YUNI_CORE_BIT_BIT_H__
# define __YUNI_CORE_BIT_BIT_H__

# include "../../yuni.h"

/*!
** \brief Get if the Nth bit is set in a raw char* buffer
*/
# define YUNI_BIT_GET(DATA,INDEX)   \
	(((const unsigned char*)(DATA))[(INDEX) >> 3] & (1 << (7 - ((INDEX) & 7))))

/*!
** \brief Set the Nth bit in a raw char* buffer
*/
# define YUNI_BIT_SET(DATA,INDEX)  \
	(((unsigned char*)(DATA))[(INDEX) >> 3] |= (unsigned char)(1 << (7 - ((INDEX) & 7))))

/*!
** \brief Unset the Nth bit in a raw char* buffer
*/
# define YUNI_BIT_UNSET(DATA,INDEX)  \
	(((unsigned char*)(DATA))[(INDEX) >> 3] &= (unsigned char)~(1 << (7 - ((INDEX) & 7))))




namespace Yuni
{
namespace Core
{
namespace Bit
{


	/*!
	** \brief Get the number of bits that are set
	*/
	template<class T> unsigned int Count(T data);

	/*!
	** \brief Get if the bit at a given index
	*/
	bool Get(const char*, unsigned int index);

	/*!
	** \brief Set the Nth bit in a raw buffer
	*/
	void Set(char* data, unsigned int index);

	/*!
	** \brief Unset the Nth bit in a raw buffer
	*/
	void Unset(char* data, unsigned int index);




} // namespace Bit
} // namespace Core
} // namespace Yuni

# include "bit.hxx"

#endif // __YUNI_CORE_BIT_BIT_H__

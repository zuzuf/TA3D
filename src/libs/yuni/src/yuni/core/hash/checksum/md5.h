#ifndef __YUNI_CORE_HASH_CHECKSUM_MD5_H__
# define __YUNI_CORE_HASH_CHECKSUM_MD5_H__

# include "../../../yuni.h"
# include "checksum.h"



namespace Yuni
{
namespace Hash
{
namespace Checksum
{


	/*!
	** \brief MD5 Checksum
	**
	** Compute the MD5 from a string :
	** \code
	** Yuni::Hash::MD5 md5;
	** md5.fromString("Hello world");
	** std::cout << md5.value() << std::endl;
	** \endcode
	**
	** Another way to do it (1/2 - recommended) :
	** \code
	** Yuni::Hash::MD5 md5;
	** std::cout << md5.fromString("Hello world") << std::endl;
	** std::cout << md5.value() << std::endl; // will remain the same than before
	** \endcode
	**
	** Another way to do it (2/2) :
	** \code
	** std::cout << Yuni::Hash::MD5::FromString("Hello world") << std::endl;
	** \endcode
	**
	** Compute a lot of MD5 from a string :
	** \code
	** MD5 md5;
	** std::cout << md5["Hello"] << std::endl;
	** std::cout << md5["World"] << std::endl;
	** std::cout << md5["Hello world"] << std::endl;
	** \endcode
	*/
	class MD5 : public Hash::Checksum::IChecksum
	{
	public:
		/*!
		** \brief Compute the hash from a string
		**
		** \param s The string
		** \return The hash value
		*/
		static String FromString(const String& s);

		/*!
		** \brief Compute the hash from raw data
		**
		** \param rawdata The original buffer
		** \param size Size of the given buffer.
		** \return The hash value
		*/
		static String FromRawData(const void* rawdata, uint64 size = AutoDetectNullChar);

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		MD5() {}
		//! Destructor
		virtual ~MD5() {}
		//@}

		virtual const String& fromRawData(const void* rawdata, uint64 size = AutoDetectNullChar);
		virtual const String& fromFile(const String& filename);

	}; // class Hash::MD5




} // namespace Checksum
} // namespace Hash
} // namespace Yuni

# include "md5.hxx"

#endif // __YUNI_CORE_HASH_CHECKSUM_MD5_H__

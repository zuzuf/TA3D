#ifndef __YUNI_CORE_HASH_CHECKSUM_CHECKSUM_H__
# define __YUNI_CORE_HASH_CHECKSUM_CHECKSUM_H__

# include "../../../yuni.h"
# include "../../string.h"


namespace Yuni
{

/*!
** \brief Tools which use hashing mechanisms
*/
namespace Hash
{

/*!
** \brief Checksums
*/
namespace Checksum
{



	/*!
	** \brief Checksum Implementation (Abstract)
	*/
	class IChecksum
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		IChecksum() {}
		//! Destructor
		virtual ~IChecksum() {}
		//@}

		/*!
		** \brief Reset the hash value
		*/
		void reset();

		/*!
		** \brief Compute the hash from a string
		**
		** \param s The string
		** \return The hash value
		*/
		const String& fromString(const String& s);

		/*!
		** \brief Compute the hash from raw data
		**
		** \param rawdata The buffer
		** \param size The size of the buffer. AutoDetectNullChar will make an autodetection of the length
		** \return The hash value
		*/
		virtual const String& fromRawData(const void* rawdata, uint64 size = AutoDetectNullChar) = 0;

		/*!
		** \brief Compute the hash of a given file
		**
		** \param filename The filename to analyze
		** \return The hash value
		*/
		virtual const String& fromFile(const String& filename) = 0;

		/*!
		** \brief Get the last hash value
		*/
		const String& value() const;
		//! Get the hash value
		const String& operator() () const;

		/*!
		** \brief Compute the hash value from a string and returns it
		**
		** \param s The string to compute
		** \return The hash value
		*/
		const String& operator[] (const String& s);

	protected:
		//! The hash value
		String pValue;

	}; // class Hash::IChecksum




} // namespace Checksum
} // namespace Hash
} // namespace Yuni

# include "checksum.hxx"

#endif // __YUNI_CORE_HASH_CHECKSUM_CHECKSUM_H__

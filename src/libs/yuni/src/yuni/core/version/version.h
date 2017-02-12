#ifndef __YUNI_CORE_VERSION_H__
# define __YUNI_CORE_VERSION_H__

# include "../../yuni.h"
# include "../string.h"



namespace Yuni
{


	/*!
	** \brief Version number
	**
	** A version number is composed by two numbers (major and minor), plus a
	** revision number for fine-grained level.
	**
	** \ingroup Core
	*/
	class Version
	{
	public:
		/*!
		** \brief Get the version of the Yuni Library
		*/
		static void InternalLib(Version& v);

	public:
		//! \name Constructor
		//@{
		//! Default constructor
		Version();

		/*!
		** \brief Constructor with a given version
		**
		** \param h The major version number
		*/
		explicit Version(unsigned int h);

		/*!
		** \brief Constructor with a given version
		**
		** \param h The major version number
		** \param l The minor version number
		*/
		Version(unsigned int h, unsigned int l);

		/*!
		** \brief Constructor with a given version
		**
		** \param h The major version number
		** \param l The minor version number
		** \param r The revision number
		*/
		Version(unsigned int h, unsigned int l, unsigned int r);

		//! Copy constructor
		Version(const Version& c);
		//@}


		//! \name Conversions
		//@{
		/*!
		** \brief Get the version in an human-readable string
		*/
		String toString() const;
		//@}


		//! \name Comparisons
		//@{
		/*!
		** \brief Get if the version is null
		*/
		bool null() const;

		/*!
		** \brief Check if this version is less than another one
		*/
		bool isLessThan(const Version& rhs) const;
		/*!
		** \brief Check if this version is equal to another one
		*/
		bool isEqualTo(const Version& rhs) const;
		/*!
		** \brief Check if this version is greater than another one
		*/
		bool isGreaterThan(const Version& rhs) const;
		//@}


		//! \name ostream
		//@{
		/*!
		** \brief Print the version to a ostream
		*/
		void print(std::ostream& out) const;
		//@}


		//! \name Operators
		//@{
		//! The operator <
		bool operator <  (const Version& rhs) const;
		//! The operator <=
		bool operator <= (const Version& rhs) const;
		//! The operator >
		bool operator >  (const Version& rhs) const;
		//! The operator <=
		bool operator >= (const Version& rhs) const;

		//! The operator ==
		bool operator == (const Version& rhs) const;
		//! The operator !=
		bool operator != (const Version& rhs) const;

		//! The operator =
		Version& operator = (const Version& rhs);
		//@}


	public:
		//! The major version number
		unsigned int hi;
		//! The minor version number
		unsigned int lo;
		//! Revision
		unsigned int revision;

	}; // class Version




} // namespace Yuni

# include "version.hxx"

#endif // __YUNI_CORE_VERSION_H__

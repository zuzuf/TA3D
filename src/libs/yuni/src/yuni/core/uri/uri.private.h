#ifndef __YUNI_CORE_URI_PRIVATE_H__
# define __YUNI_CORE_URI_PRIVATE_H__

# include "../string.h"



namespace Yuni
{
namespace Private
{
namespace Uri
{


	/*!
	** \brief Carrier for informations about an URI
	*/
	class Informations
	{
	public:
		//! \name Constructors
		//@{
		//! Default constructor
		Informations();
		//! Copy constructor
		Informations(const Informations& rhs);
		//! Destructor
		~Informations();
		//@}

		/*!
		** \brief Assign values from another struct `Informations`
		** \param rhs Another instance
		*/
		void assign(const Informations& rhs);

		/*!
		** \brief Clear all data
		*/
		void clear();

		/*!
		** \brief Convert into a mere string
		*/
		String toString() const;

		/*!
		** \brief Get if equals to another instance
		*/
		bool isEqualsTo(const Informations& rhs) const;

		/*!
		** \brief Print the uri to a stream
		*/
		void print(std::ostream& out) const;

	public:
		//! Scheme
		String scheme;
		//! User
		String user;
		//! password
		String password;
		//! server
		String server;
		//! port
		sint32 port;
		//! path
		String path;
		//! query
		String query;
		//! fragment
		String fragment;
		//! Were the informations of the URI valid during the last build
		bool isValid;

	}; // class Informations




} // namespace Uri
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_URI_PRIVATE_H__

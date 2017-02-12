#ifndef __YUNI_NET_PORT_H__
# define __YUNI_NET_PORT_H__

# include "../yuni.h"


namespace Yuni
{
namespace Net
{

	/*!
	** \brief Socket Port number
	**
	** \see http://www.iana.org/assignments/port-numbers
	*/
	class Port
	{
	public:
		//! \name Constructors
		//@{
		//! Default constructor
		Port();
		//! Constructor with a given value
		Port(unsigned int rhs);
		//! Copy constructor
		Port(const Port& rhs);
		//! Constructor from a null pointer
		Port(const NullPtr&);
		//@}

		//! \name Value
		//@{
		//! Get the port number
		unsigned int value() const;
		//! Get if the port is valid
		bool valid() const;
		//@}

		//! \name Ranges
		//@{
		//! Get if the port is well-known (0..1023)
		bool wellKnown() const;

		//! Get if the port is registered (0..49151)
		bool registered() const;

		//! Get if the port is dynamic and/or private (>= 49152)
		bool dynamic() const;

		//! Get if no port is allocated
		bool none() const;
		//@}

		//! \name Operators
		//@{
		//! Assignment
		Port& operator = (unsigned int rhs);
		Port& operator = (const Port& rhs);
		Port& operator = (const Yuni::NullPtr&);
		//! Append
		Port& operator += (unsigned int rhs);
		//! Sub
		Port& operator -= (unsigned int rhs);
		//! Comparison
		bool operator == (unsigned int rhs) const;
		bool operator == (const Port& rhs) const;
		bool operator != (unsigned int rhs) const;
		bool operator != (const Port& rhs) const;
		bool operator ! () const;
		//! Inequality
		bool operator < (const Port& rhs) const;
		bool operator > (const Port& rhs) const;
		bool operator <= (const Port& rhs) const;
		bool operator >= (const Port& rhs) const;
		//@}

	private:
		//! Port value
		unsigned int pValue;

	}; // class Port






} // namespace Net
} // namespace Yuni

# include "port.hxx"

#endif // __YUNI_NET_PORT_H__

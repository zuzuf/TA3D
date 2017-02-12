#ifndef __YUNI_NET_PORT_HXX__
# define __YUNI_NET_PORT_HXX__


namespace Yuni
{
namespace Net
{

	inline Port::Port()
		:pValue(0)
	{}


	inline Port::Port(unsigned int rhs)
		:pValue(rhs)
	{}


	inline Port::Port(const Port& rhs)
		:pValue(rhs.pValue)
	{}


	inline Port::Port(const NullPtr&)
		:pValue(0)
	{}


	inline bool Port::wellKnown() const
	{
		return pValue < 1024u;
	}


	inline bool Port::registered() const
	{
		return (pValue >= 1024) && (pValue <= 49151);
	}


	inline bool Port::dynamic() const
	{
		return pValue > 49151;
	}


	inline unsigned int Port::value() const
	{
		return pValue;
	}


	inline bool Port::valid() const
	{
		return pValue && pValue <= 65535;
	}


	inline bool Port::none() const
	{
		return !pValue;
	}


	inline Port& Port::operator = (unsigned int rhs)
	{
		pValue = rhs;
		return *this;
	}


	inline Port& Port::operator = (const Port& rhs)
	{
		pValue = rhs.pValue;
		return *this;
	}


	inline Port& Port::operator = (const NullPtr&)
	{
		pValue = 0;
		return *this;
	}


	inline Port& Port::operator += (unsigned int rhs)
	{
		pValue += rhs;
		return *this;
	}


	inline Port& Port::operator -= (unsigned int rhs)
	{
		pValue -= rhs;
		return *this;
	}


	inline bool Port::operator == (unsigned int rhs) const
	{
		return rhs == pValue;
	}


	inline bool Port::operator == (const Port& rhs) const
	{
		return rhs.pValue == pValue;
	}


	inline bool Port::operator != (unsigned int rhs) const
	{
		return rhs != pValue;
	}


	inline bool Port::operator != (const Port& rhs) const
	{
		return rhs.pValue != pValue;
	}


	inline bool Port::operator ! () const
	{
		return ! pValue;
	}


	inline bool Port::operator < (const Port& rhs) const
	{
		return pValue < rhs.pValue;
	}


	inline bool Port::operator > (const Port& rhs) const
	{
		return pValue > rhs.pValue;
	}


	inline bool Port::operator <= (const Port& rhs) const
	{
		return pValue <= rhs.pValue;
	}


	inline bool Port::operator >= (const Port& rhs) const
	{
		return pValue >= rhs.pValue;
	}






} // namespace Net
} // namespace Yuni

#endif // __YUNI_NET_PORT_HXX__

#ifndef __YUNI_CORE_VARIANT_VARIANT_PRIVATE_H__
# define __YUNI_CORE_VARIANT_VARIANT_PRIVATE_H__

# include <sstream>
# include "../static/types.h"
# include "../math/math.h"


namespace Yuni
{
namespace Private
{
namespace Variant
{

	/*!
	** \brief Data Converter interface (Interface)
	*/
	struct IDataConverter
	{
		//! Destructor
		virtual ~IDataConverter()
		{}

		//! \name From- converters for base types
		//@{
		virtual bool convertFrom(const bool v) = 0;
		virtual bool convertFrom(const int v) = 0;
		virtual bool convertFrom(const float v) = 0;
		virtual bool convertFrom(const double v) = 0;
		virtual bool convertFrom(const String& v) = 0;
		//@}

	}; // class IDataConverter



	/*!
	** \brief The real convertor structure.
	*/
	template <typename From, typename To>
	struct Converter
	{
		static bool Value(const From& from, To& to)
		{
			to = static_cast<To>(from);
			return true;
		}
	};

	// Specialization to avoid warning from Visual Studio (C4800)
	template <typename From>
	struct Converter<From, bool>
	{
		static bool Value(const From& from, bool& to)
		{
			to = Math::Equals(From(), from);
			return true;
		}
	};


	/*!
	** \brief Concrete DataConverter implementation
	*/
	template <typename TargetType>
	struct DataConverter : public IDataConverter
	{
	public:
		DataConverter() : result()
		{}

		virtual bool convertFrom(const bool v)
		{ return Converter<bool,TargetType>::Value(v, result); }

		virtual bool convertFrom(const int v)
		{ return Converter<int,TargetType>::Value(v, result); }

		virtual bool convertFrom(const float v)
		{ return Converter<float,TargetType>::Value(v, result); }

		virtual bool convertFrom(const double v)
		{ return Converter<double,TargetType>::Value(v, result); }

		virtual bool convertFrom(const String& v)
		{ result = v.to<TargetType>(); return true; }

	public:
		//! The conversion Result
		TargetType result;

	}; // class DataConverter



	/*!
	** \brief Abstract container for variant data.
	*/
	class AData
	{
	public:
		//! Constructor
		AData()
		{}

		//! Destructor
		virtual ~AData()
		{}

		//! Converts the data to the type T.
		template <typename T>
		T to() const;

		/*!
		** \brief Clones the enclosed data
		** \returns A pointer on the new instance
		*/
		virtual AData* clone() const = 0;

	protected:
		/*!
		** \brief Runs the conversion using the specified converter.
		** The result of the conversion can be retrieved from
		** the converter itself.
		** \param[in] A reference on an instancied DataConverter<DestinationType>.
		*/
		virtual bool convertUsing(IDataConverter& cvtr) const = 0;

	}; // class AData




	/*!
	** \brief Concrete variant data container
	**
	** This is templated with the real data type.
	*/
	template <typename T>
	class Data : public AData
	{
	public:
		//! Constructor from the variable type
		explicit Data(const T& src)
			:pValue(src)
		{}

		//! Destructor
		virtual ~Data()
		{}

	protected:
		virtual bool convertUsing(IDataConverter& cvtr) const
		{ return cvtr.convertFrom(pValue); }

		virtual AData* clone() const
		{ return new Data<T>(pValue); }

	private:
		//! The real data element.
		T pValue;

	}; // class Data




} // namespace Variant
} // namespace Private
} // namespace Yuni


#include "variant.private.hxx"

#endif /* !__YUNI_CORE_VARIANT_VARIANT_PRIVATE_H__ */

#ifndef __YUNI_DEVICE_DISPLAY_RESOLUTION_H__
# define __YUNI_DEVICE_DISPLAY_RESOLUTION_H__

# include "../../yuni.h"
# include <vector>
# include <list>
# include "../../core/string.h"
# include "../../core/smartptr/smartptr.h"



namespace Yuni
{
namespace Device
{
namespace Display
{


	/*!
	** \brief Screen/Monitor resolution
	*/
	class Resolution
	{
	public:
		//! The most suitable smart pointer to use with the class `Resolution`
		typedef SmartPtr<Resolution> Ptr;

		enum
		{
			//! The smallest value allowed for the width of the screen
			minimumWidth  = 320u,
			//! The smallest allowed value for the height of the screen
			minimumHeight = 200u,

			//! The highest allowed for the width of the screen
			maximumWidth  = 2560u,
			//! The highest allowed value for the height of the screen
			maximumHeight = 2048u,
		};

		//! Vector of resolutions
		typedef std::vector<Ptr>  Vector;

	public:
		//! \name Constructors
		//@{
		/*!
		** \brief Constructor
		**
		** \param w Width of the monitor/screen
		** \param h Height of the monitor/screen
		** \param b Bit per Pixel
		*/
		Resolution(const uint32 w, const uint32 h, const uint8 b = 32);

		/*!
		** \brief Constructor by copy
		** \param c The instance to copy
		*/
		Resolution(const Resolution& c);
		//@}

		/*!
		** \brief Convert these informations into an human readable string
		*/
		String toString() const;


		//! \name Information about the current mode
		//@{
		//! The width of the monitor/screen
		uint32 width() const;
		//! The height of the monitor/screen
		uint32 height() const;
		//! Bit per pixel
		uint8 bitPerPixel() const;
		//@}


		//! \name Operators
		//@{
		/*!
		** \brief Comparison operator (equal with)
		**
		** \param rhs The other resolution to compare with
		** \return True if the two resolution are equal
		*/
		bool operator == (const Resolution& rhs) const;

		/*!
		** \brief Comparison operator (non equal with)
		**
		** \param rhs The other resolution to compare with
		** \return True if the two resolution are not equal
		*/
		bool operator != (const Resolution& rhs) const;

		/*!
		** \brief Comparison operator (less than)
		**
		** \param rhs The other resolution to compare with
		** \return True if *this < rhs
		*/
		bool operator < (const Resolution& rhs) const;

		/*!
		** \brief Comparison operator (less than or equal)
		**
		** \param rhs The other resolution to compare with
		** \return True if *this <= rhs
		*/
		bool operator <= (const Resolution& rhs) const;

		/*!
		** \brief Comparison operator (greater than)
		**
		** \param rhs The other resolution to compare with
		** \return True if *this > rhs
		*/
		bool operator > (const Resolution& rhs) const;

		/*!
		** \brief Comparison operator (greater than or equal)
		**
		** \param rhs The other resolution to compare with
		** \return True if *this >= rhs
		*/
		bool operator >= (const Resolution& rhs) const;

		/*!
		** \brief Assign new values from another resolution
		**
		** \param p The new values
		** \return Always *this
		*/
		Resolution& operator = (const Resolution& p);
		//@}


		//! \name Stream printing
		//@{
		/*!
		** \brief Print the resolution
		**
		** \param[in,out] out An output stream
		** \return The output stream `out`
		*/
		std::ostream& print(std::ostream& out) const;
		//@}

	private:
		//! Height of the screen
		uint32 pWidth;
		//! Width of the screen
		uint32 pHeight;
		//! Bits per pixel
		uint8 pBitsPerPixel;

	}; // class Resolution





} // namespace Display
} // namespace Device
} // namespace Yuni

# include "resolution.hxx"


//! \name Operator overload for stream printing
//@{
inline std::ostream& operator << (std::ostream& out, const Yuni::Device::Display::Resolution& rhs)
{ return rhs.print(out); }
inline std::ostream& operator << (std::ostream& out, const Yuni::Device::Display::Resolution::Ptr& rhs)
{ return rhs->print(out); }
//@}


#endif // __YUNI_DEVICE_DISPLAY_RESOLUTION_H__

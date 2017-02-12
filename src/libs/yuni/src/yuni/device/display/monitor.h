#ifndef __YUNI_DEVICE_DISPLAY_MONITOR_H__
# define __YUNI_DEVICE_DISPLAY_MONITOR_H__

# include "../../yuni.h"
# include "resolution.h"
# include "../../core/string.h"
# include "../../core/smartptr/smartptr.h"


# if defined(YUNI_OS_MACOSX)
#	define YUNI_DEVICE_DISPLAY_HANDLE         uint32
#	define YUNI_DEVICE_DISPLAY_INVALIDHANDLE  (uint32(-1))
# else
# 	if defined(YUNI_OS_WINDOWS)
#		define YUNI_DEVICE_DISPLAY_HANDLE         uint32
#		define YUNI_DEVICE_DISPLAY_INVALIDHANDLE  (uint32(-1))
# 	else
/* Unixes - X11 */
#		define YUNI_DEVICE_DISPLAY_HANDLE         int
#		define YUNI_DEVICE_DISPLAY_INVALIDHANDLE  (int(-1))
# 	endif
# endif



namespace Yuni
{
namespace Device
{
namespace Display
{


	/*!
	** \brief Information about a single Frame Buffer Device
	**
	** This class is not thread-safe
	*/
	class Monitor
	{
	public:
		//! The most suitable smart pointer to use with the class `Monitor`
		typedef SmartPtr<Monitor> Ptr;

		/*!
		** \brief Handle for a single monitor
		**
		** This handle is system-dependant
		** <ul>
		**    <li>Mac OS : The equivalent is `CGDirectDisplayID`</li>
		** </ul>
		*/
		typedef YUNI_DEVICE_DISPLAY_HANDLE  Handle;
		//! Value for an invalid handle
		static const Handle InvalidHandle = YUNI_DEVICE_DISPLAY_INVALIDHANDLE;


	public:
		//! \name Constructors and Destructor
		//@{
		/*!
		** \brief Default constructor
		**
		** By default, some safe (assumed) resolutions will be available
		*/
		Monitor();

		/*!
		** \brief Constructor with values
		**
		** By default, some safe (assumed) resolutions will be available
		**
		** \param nm Name of the monitor
		** \param hwn Handle of the monitor
		** \param p Is the primary display
		** \param a Is hardware-accelerated
		** \param b Is a builtin device
		*/
		Monitor(const String& nm, const Monitor::Handle hwn = InvalidHandle, const bool p = false,
				const bool a = false, const bool b = false);

		//! Constructor by copy
		Monitor(const Monitor& c);

		//! Destructor
		~Monitor();
		//@}


		//! \name Handle
		//@{
		/*!
		** \brief Get the index of the current selected monitor
		*/
		Monitor::Handle handle() const;

		/*!
		** \brief Get if the handle of this monitor is valid
		*/
		bool valid() const;
		//@}


		//! \name Identifiers
		//@{
		/*!
		** \brief Get the human readable name of the monitor, if any
		**
		** \return The name of the monitor (product), or an empty value if not available
		*/
		const String& productName() const;

		/*!
		** \brief Get an unique id for this screen
		**
		** This ID should be considered as unique only on the local computer.
		** This guid may be useful to load/save settings according the current monitor
		**
		** \return A md5 string
		*/
		const String& guid();
		//@}

		//! \name Informations about the monitor
		//@{
		/*!
		** \brief Get all available resolutions for this screen
		**
		** The returned value is guaranteed to not be empty and to be
		** a sorted descendant list.
		*/
		const Resolution::Vector& resolutions() const;

		/*!
		** \brief Get the recommended resolution for this device
		**
		** It is merely the highest available resolution
		** \return A valid and not null resolution
		*/
		Resolution::Ptr recommendedResolution() const;

		/*!
		** \brief Remove all resolutions
		*/
		void clear();

		/*!
		** \brief Get if a resolution is valid for this monitor
		**
		** \param rhs The resolution to check
		** \return True if the resolution is valid, merely if this resolution is in the list
		*/
		bool resolutionIsValid(const Resolution::Ptr& rhs) const;

		/*!
		** \brief Get if this monitor is the primary display
		*/
		bool primary() const;

		/*!
		** \brief Get if the device is hardware accelerated
		*/
		bool hardwareAcceleration() const;

		/*!
		** \brief Get if the monitor is a builtin device
		**
		** \note Only available on Mac OS X v10.2 or later
		*/
		bool builtin() const;
		//@}

		/*!
		** \brief Add a new resolution in the list
		**
		** \param[in] r The resolution to add
		** \internal It is a sorted descendant list. The first value must be the highest available value
		*/
		void add(const Resolution::Ptr& r);

		/*!
		** \brief Add some safe resolutions
		*/
		void addSafeResolutions();

		//! \name Operators
		//@{
		//! Append a resolution
		Monitor& operator += (Resolution* rhs);
		//! Append a resolution
		Monitor& operator += (const Resolution::Ptr& rhs);
		//! Append a resolution
		Monitor& operator << (Resolution* rhs);
		//! Append a resolution
		Monitor& operator << (const Resolution::Ptr& rhs);
		//@}

	private:
		void internalAppendSafeResolutions();

	protected:
		//! The index of the monitor
		Monitor::Handle pHandle;
		//! Name of the current monitor
		String pProductName;
		/*!
		** \brief All resolutions
		** \internal It is a sorted descendant list. The first value must be the highest available value
		*/
		Resolution::Vector pResolutions;
		//! Primary
		bool pPrimary;
		//! Hardware Acceleration
		bool pHardwareAcceleration;
		//! Builtin device
		bool pBuiltin;

		//! The MD5 for the guid - avoid multiple md5 calculations
		String pMD5Cache;

	}; // class Monitor




} // namespace Display
} // namespace Device
} // namespace Yuni

# include "monitor.hxx"

#endif // __YUNI_DEVICE_DISPLAY_MONITOR_H__

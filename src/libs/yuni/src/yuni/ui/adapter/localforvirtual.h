#ifndef __YUNI_UI_ADAPTER_LOCALFORVIRTUAL_H__
# define __YUNI_UI_ADAPTER_LOCALFORVIRTUAL_H__

# include "../component.h"
# include "forvirtual.h"

namespace Yuni
{

namespace Private
{
namespace UI
{
namespace Local
{
namespace Adapter
{

	// Forward declaration
	class LocalForRepresentation;

} // namespace Adapter
} // namespace Local
} // namespace UI
} // namespace Private


namespace UI
{
namespace Adapter
{


	/*!
	** \brief A local adapter for communication between Yuni::UI and Yuni::Private::UI::Local
	*/
	class LocalForVirtual: public ForVirtual
	{
	public:
		LocalForVirtual();
		LocalForVirtual(Private::UI::Local::Adapter::LocalForRepresentation* forRepresentation);

		virtual void sendShowWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const;
		virtual void sendHideWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const;
		virtual void sendCloseWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const;
	};


} // namespace Adapter
} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_ADAPTER_LOCALFORVIRTUAL_H__

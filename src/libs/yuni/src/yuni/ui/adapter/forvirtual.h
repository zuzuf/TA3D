#ifndef __YUNI_UI_ADAPTER_FORVIRTUAL_H__
# define __YUNI_UI_ADAPTER_FORVIRTUAL_H__

# include "../../yuni.h"
# include "../component.h"
# include "../fwd.h"

namespace Yuni
{
namespace UI
{
namespace Adapter
{


	/*!
	** \brief Interface for adapters on the virtual UI side
	*/
	class ForVirtual
	{
	public:
		ForVirtual();
		ForVirtual(Private::UI::Local::Adapter::ForRepresentation* forRepresentation);

	public:
		virtual void sendShowWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const = 0;
		virtual void sendHideWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const = 0;
		virtual void sendCloseWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const = 0;

	protected:
		Private::UI::Local::Adapter::ForRepresentation* pDestination;
	};


} // namespace Adapter
} // namespace UI
} // namespace Yuni

#endif // __YUNI_UI_ADAPTER_FORVIRTUAL_H__

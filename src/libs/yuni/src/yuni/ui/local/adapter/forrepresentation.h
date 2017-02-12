#ifndef __YUNI_PRIVATE_UI_LOCAL_ADAPTER_FORREPRESENTATION_H__
# define __YUNI_PRIVATE_UI_LOCAL_ADAPTER_FORREPRESENTATION_H__

# include "../../../yuni.h"
# include "../../component.h"
# include "../../fwd.h"

namespace Yuni
{

namespace UI
{
namespace Adapter
{

	// Forward declaration
	class ForVirtual;

} // namespace Adapter
} // namespace UI


namespace Private
{
namespace UI
{
namespace Local
{
namespace Adapter
{


	/*!
	** \brief Interface for adapters on the host-dependent representation side
	*/
	class ForRepresentation
	{
	public:
		ForRepresentation();
		ForRepresentation(Yuni::UI::Adapter::ForVirtual* forVirtual);
		virtual ~ForRepresentation();

	public:
		virtual void receiveShowWindow(const Yuni::UI::GUID& applicationID, Yuni::UI::IComponent::ID id) const = 0;
		virtual void receiveHideWindow(const Yuni::UI::GUID& applicationID, Yuni::UI::IComponent::ID id) const = 0;
		virtual void receiveCloseWindow(const Yuni::UI::GUID& applicationID, Yuni::UI::IComponent::ID id) const = 0;

	protected:
		Yuni::UI::Adapter::ForVirtual* pDestination;
	};


} // namespace Adapter
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_PRIVATE_UI_LOCAL_ADAPTER_FORREPRESENTATION_H__

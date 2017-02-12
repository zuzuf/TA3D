#ifndef __YUNI_PRIVATE_UI_LOCAL_ADAPTER_LOCALFORREPRESENTATION_H__
# define __YUNI_PRIVATE_UI_LOCAL_ADAPTER_LOCALFORREPRESENTATION_H__

# include "forrepresentation.h"

namespace Yuni
{

namespace UI
{
namespace Adapter
{

	// Forward declaration
	class LocalForVirtual;

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
	** \brief A local adapter for communication between Yuni::UI and Yuni::Private::UI::Local
	*/
	class LocalForRepresentation: public ForRepresentation
	{
	public:
		LocalForRepresentation();
		LocalForRepresentation(Yuni::UI::Adapter::LocalForVirtual* forVirtual);
		virtual ~LocalForRepresentation();

	public:
		virtual void receiveShowWindow(const Yuni::UI::GUID& applicationID, Yuni::UI::IComponent::ID id) const;
		virtual void receiveHideWindow(const Yuni::UI::GUID& applicationID, Yuni::UI::IComponent::ID id) const;
		virtual void receiveCloseWindow(const Yuni::UI::GUID& applicationID, Yuni::UI::IComponent::ID id) const;
	};


} // namespace Adapter
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_PRIVATE_UI_LOCAL_ADAPTER_LOCALFORREPRESENTATION_H__

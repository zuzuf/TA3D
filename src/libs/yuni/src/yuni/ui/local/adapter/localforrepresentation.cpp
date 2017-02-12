
#include "localforrepresentation.h"
#include "../../adapter/localforvirtual.h"

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

	LocalForRepresentation::LocalForRepresentation()
	{}


	LocalForRepresentation::LocalForRepresentation(Yuni::UI::Adapter::LocalForVirtual* forVirtual):
		ForRepresentation(forVirtual)
	{}

	LocalForRepresentation::~LocalForRepresentation()
	{}


	void LocalForRepresentation::receiveShowWindow(const Yuni::UI::GUID& /*applicationID*/, Yuni::UI::IComponent::ID /*id*/) const
	{
		// TODO : get the window, ask it to show
	}


	void LocalForRepresentation::receiveHideWindow(const Yuni::UI::GUID& /*applicationID*/, Yuni::UI::IComponent::ID /*id*/) const
	{
		// TODO : get the window, ask it to hide
	}


	void LocalForRepresentation::receiveCloseWindow(const Yuni::UI::GUID& /*applicationID*/, Yuni::UI::IComponent::ID /*id*/) const
	{
		// TODO : get the window, ask it to close
	}





} // namespace Adapter
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

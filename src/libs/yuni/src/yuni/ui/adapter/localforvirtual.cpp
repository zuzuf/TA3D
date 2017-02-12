
#include "localforvirtual.h"
#include "../local/adapter/forrepresentation.h"
#include "../local/adapter/localforrepresentation.h"

namespace Yuni
{
namespace UI
{
namespace Adapter
{

	LocalForVirtual::LocalForVirtual()
	{}


	LocalForVirtual::LocalForVirtual(Private::UI::Local::Adapter::LocalForRepresentation* forRepresentation):
		ForVirtual(forRepresentation)
	{}


	void LocalForVirtual::sendShowWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const
	{
		if (pDestination)
			pDestination->receiveShowWindow(applicationID, id);
	}

	void LocalForVirtual::sendHideWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const
	{
		if (pDestination)
			pDestination->receiveHideWindow(applicationID, id);
	}

	void LocalForVirtual::sendCloseWindow(const Yuni::UI::GUID& applicationID, IComponent::ID id) const
	{
		if (pDestination)
			pDestination->receiveCloseWindow(applicationID, id);
	}


} // namespace Adapter
} // namespace UI
} // namespace Yuni

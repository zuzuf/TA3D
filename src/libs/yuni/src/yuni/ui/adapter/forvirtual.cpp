
#include "forvirtual.h"

namespace Yuni
{
namespace UI
{
namespace Adapter
{

	ForVirtual::ForVirtual():
		pDestination(nullptr)
	{}


	ForVirtual::ForVirtual(Private::UI::Local::Adapter::ForRepresentation* forRepresentation):
		pDestination(forRepresentation)
	{}


} // namespace Adapter
} // namespace UI
} // namespace Yuni


#include "forrepresentation.h"
#include "../../adapter/forvirtual.h"

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

	ForRepresentation::ForRepresentation():
		pDestination(nullptr)
	{}

	ForRepresentation::ForRepresentation(Yuni::UI::Adapter::ForVirtual* forVirtual):
		pDestination(forVirtual)
	{}

	ForRepresentation::~ForRepresentation()
	{}



} // namespace Adapter
} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

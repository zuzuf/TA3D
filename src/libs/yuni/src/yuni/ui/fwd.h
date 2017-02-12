#ifndef __YUNI_UI_FWD_H__
# define __YUNI_UI_FWD_H__


namespace Yuni
{

namespace UI
{

	//! Forward declaration
	class IControlContainer;
	class Application;
	class Desktop;
	class Window;

	//! String identifier type
	typedef CustomString<40, false, false> GUID;


namespace Adapter
{

	//! Forward declaration
	class ForVirtual;
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

	// Forward declaration
	class ForRepresentation;
	class LocalForRepresentation;

} // namespace Adapter
} // namespace Local
} // namespace UI
} // namespace Private


} // namespace Yuni

#endif // __YUNI_UI_FWD_H__

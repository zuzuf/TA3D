#ifndef __YUNI_UI_LOCAL_QUEUESERVICE_H__
# define __YUNI_UI_LOCAL_QUEUESERVICE_H__

# include "../../yuni.h"
# include "window.h"
# include "adapter/forrepresentation.h"


namespace Yuni
{
namespace Private
{
namespace UI
{
namespace Local
{

	/*!
	** \brief Main loop for UI
	*/
	template<class ChildT>
    class IQueueService :
		public Core::EventLoop::IEventLoop<ChildT,
			Core::EventLoop::Flow::Continuous, Core::EventLoop::Statistics::None, false>,
		public IEventObserver<IQueueService<ChildT> >
	{
	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		IQueueService();
		//! Destructor
		~IQueueService();
		//@}


		//! Create a local system-dependent window for UI
		IWindow::Ptr createWindow();


		//! Destroy a local system-dependent window
		void destroyWindow(IWindow::Ptr& window);

	protected:
		//! Map of all local windows
		IWindow::Map  pWindows;

	}; // class IQueueService<>





} // namespace Local
} // namespace UI
} // namespace Private
} // namespace Yuni

#endif // __YUNI_UI_LOCAL_QUEUESERVICE_H__

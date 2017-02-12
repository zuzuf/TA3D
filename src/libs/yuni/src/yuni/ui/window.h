#ifndef __YUNI_UI_WINDOW_H__
# define __YUNI_UI_WINDOW_H__

# include "../yuni.h"
# include "../core/string.h"
# include "../core/event/event.h"
# include "../thread/policy.h"
# include "fwd.h"
# include "control/controlcontainer.h"


namespace Yuni
{
namespace UI
{

	/*!
	** \brief Abstraction of a window for graphic rendering
	*/
	class Window: public IControlContainer
	{
	public:
		//! Smart pointer, inherited from the ancestor
		typedef IComponent::SmartPtrInfo<Window>::Type Ptr;
		//! Vector of controls
		typedef std::vector<Ptr> Vector;
		//! Map of controls
		typedef std::map<IComponent::ID, Ptr> Map;
		//! Threading Policy
		typedef IComponent::ThreadingPolicy ThreadingPolicy;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Short constructor
		*/
		template<class StringT> explicit Window(const StringT& title);

		/*!
		** \brief Constructor with dimensions
		*/
		template<class StringT>
		Window(const StringT& title, float width, float height);

		/*!
		** \brief Constructor with start position coordinates
		*/
		template<class StringT>
		Window(const StringT& title, float x, float y, float width, float height);

		/*!
		** \brief Constructor with start position as a point
		*/
		template<class StringT, class T>
		Window(const StringT& title, const Point2D<T>& pos, float width, float height);

		//! Virtual destructor
		virtual ~Window();
		//@}


		//! \name Methods
		//@{
		/*!
		** \brief Show the window
		*/
		void show();

		/*!
		** \brief Hide the window
		*/
		void hide();

		/*!
		** \brief Close the window, release everything
		*/
		void close();

		/*!
		** \brief Get whether the window is in the process of closing
		*/
		bool closing() const;
		//@}


		//! \name Title of the Window
		//@{
		//! Get the Title of the window
		String title() const;
		//! Set the title of the window
		template<class StringT> void title(const StringT& newTitle);
		//@}


	protected:
		//! Set the title of the window (without lock)
		template<class StringT> void titleWL(const StringT& newTitle);

		/*!
		** \brief Resize the window
		*/
		virtual void resizeWL(float& width, float& height);

		/*!
		** \brief Update the given component's local representation
		**
		** When updating a component, reaching a window means we got to the top of the UI tree.
		** The process will now propagate the update notification to the application.
		*/
		virtual void updateComponentWL(const IComponent::ID& componentID) const;

	private:
		//! Title of the window
		String pTitle;
		//! Is the window currently closing ?
		bool pClosing;
		//! GUID of the associated application
		GUID pApplicationGUID;

		//! Friend: required for access to events
		friend class Application;

	}; // class Window






} // namespace UI
} // namespace Yuni

# include "application.h"
# include "window.hxx"

#endif // __YUNI_UI_WINDOW_H__

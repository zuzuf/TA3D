#ifndef __YUNI_UI_CONTROL_BUTTON_H__
# define __YUNI_UI_CONTROL_BUTTON_H__

# include "../../yuni.h"
# include "../../thread/policy.h"
# include "../../core/string.h"
# include "control.h"



namespace Yuni
{
namespace UI
{

	/*!
	** \brief Base class for all UI controls (viewable components)
	*/
	class Button : public IControl
	{
	public:
   		//! Smart pointer
		typedef IComponent::SmartPtrInfo<Button>::Type Ptr;
		//! Vector of controls
		typedef std::vector<Ptr> Vector;
		//! Threading Policy
		typedef IComponent::ThreadingPolicy ThreadingPolicy;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Empty constructor
		*/
		Button();

		/*!
		** \brief Constructor with parent
		*/
		Button(const IControl::Ptr& parent);

		/*!
		** \brief Constructor with caption
		*/
		template<class StringT>
		explicit Button(const StringT& caption);

		/*!
		** \brief Constructor with parent and caption
		*/
		template<class StringT>
		Button(const IControl::Ptr& parent, const StringT& caption);

		/*!
		** \brief Constructor with dimensions
		*/
		template<class StringT>
		Button(const StringT& caption, float width, float height);

		/*!
		** \brief Constructor with parent and dimensions
		*/
		template<class StringT>
		Button(const IControl::Ptr& parent, const StringT& caption, float width, float height);

		/*!
		** \brief Full constructor
		*/
		template<class StringT>
		Button(const StringT& caption, float x, float y, float width, float height);

		/*!
		** \brief Full constructor with parent
		*/
		template<class StringT>
		Button(const IControl::Ptr& parent, const StringT& caption, float x, float y,
			float width, float height);

		/*!
		** \brief Full constructor
		*/
		template<class StringT, class T>
		Button(const StringT& caption, Point2D<T>& pos, float width, float height);

		/*!
		** \brief Full constructor with parent
		*/
		template<class StringT, class T>
		Button(const IControl::Ptr& parent, const StringT& caption, Point2D<T>& pos,
			float width, float height);

		//! Virtual destructor
		virtual ~Button();
		//@}


		//! \name Caption
		//@{
		//! Get the caption of the button
		String caption() const;
		//! Set the caption
		template<class StringT> void caption(const StringT& value);
		//@}


	protected:
		//! Text caption on the button
		String pCaption;

	}; // class Button






} // namespace UI
} // namespace Yuni

# include "button.hxx"

#endif // __YUNI_UI_CONTROL_BUTTON_H__

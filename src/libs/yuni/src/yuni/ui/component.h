#ifndef __YUNI_UI_COMPONENT_H__
# define __YUNI_UI_COMPONENT_H__

# include "../yuni.h"
# include "../core/smartptr.h"
# include "../core/event.h"
# include "../thread/policy.h"
# include "../core/point2D.h"
# include "../core/string.h"
# include "id.h"

namespace Yuni
{
namespace UI
{

namespace Adapter
{
	//! Forward declaration
	class ForVirtual;

} // namespace Adapter


	/*!
	** \brief Base class for all UI components
	**
	** Defines dimension and position of the component,
	** and various common behaviours.
	*/
	class IComponent
		: public IEventObserver<IComponent, Policy::ObjectLevelLockable>
	{
	public:
		template <class T>
		struct SmartPtrInfo
		{
			//! A thread-safe node type
			typedef SmartPtr<T, Policy::Ownership::COMReferenceCounted>  Type;
		};

		typedef SmartPtrInfo<IComponent>::Type  Ptr;

		//! Ancestor type
		typedef IEventObserver<IComponent, Policy::ObjectLevelLockable> EventObserverType;
		//! Threading policy
		typedef EventObserverType::ThreadingPolicy ThreadingPolicy;

		//! A class name is a string tag representing a type of component
		typedef CustomString<64, false, false> ClassName;

		//! Unique local identifier
		typedef Yuni::UI::ID::Type ID;


	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Empty constructor
		*/
		IComponent();

		/*!
		** \brief Constructor with dimensions
		*/
		IComponent(float width, float height);

		/*!
		** \brief Full constructor
		*/
		IComponent(float x, float y, float width, float height);

		/*!
		** \brief Full constructor
		*/
		template<class T>
		IComponent(const Point2D<T>& pos, float width, float height);


		//! Virtual destructor
		virtual ~IComponent();
		//@}


		//! \name Methods
		//@{
		/*!
		** \brief Resize the component
		*/
		void resize(float width, float height);
		//@}


		//! \name Accessors
		//@{
		/*!
		** \brief Get the class name (identifier for the type of component)
		*/
		const ClassName& className() const
		{
			return pClass;
		}

		/*!
		** \brief Set the class name (identifier for the type of component)
		*/
		virtual void className(ClassName&)
		{
			pClass.clear(); // Invalid
		}

		//! Get the unique Identifier for this component
		ID id();

		/*!
		** \brief Get the width of the component
		*/
		float width() const;

		/*!
		** \brief Get the height of the component
		*/
		float height() const;

		/*!
		** \brief Get the position of the component
		*/
		Point2D<float> position() const;

		/*!
		** \brief Get the size of the component
		*/
		void size(float& width, float& height) const;

		//! Get the X position of the component
		float x() const;

		//! Get the Y position of the component
		float y() const;
		//@}


		//! \name Pointer management
		//@{
		void addRef();
		void release();
		//@}


	protected:
		//! Protected resize, without locks
		virtual void resizeWL(float& newWidth, float& newHeight);
		//! Detach the component from the tree
		virtual void detachWL();

	protected:
		/*!
		** \brief Unique local identifier
		*/
		const ID pLocalID;

		/*!
		** \brief Class name for this component (type of component)
		*/
		ClassName pClass;

		/*!
		** \brief Adapter for Virtual UI <-> UI Representation communication
		*/
		Adapter::ForVirtual* pAdapter;

		/*!
		** \brief Position of the component relative to its parent
		*/
		Point2D<float> pPosition;

		/*!
		** \brief Width of the component
		*/
		float pWidth;

		/*!
		** \brief Height of the component
		*/
		float pHeight;

	private:
		//! Intrusive reference count
		ThreadingPolicy::Volatile<int>::Type pRefCount;

	}; // class IComponent





} // namespace UI
} // namespace Yuni

# include "component.hxx"

#endif // __YUNI_GFX_UI_COMPONENT_H__

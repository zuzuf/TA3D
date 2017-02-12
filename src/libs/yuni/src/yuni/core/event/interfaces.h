#ifndef __YUNI_CORE_EVENT_INTERFACES_H__
# define __YUNI_CORE_EVENT_INTERFACES_H__

# include "../../yuni.h"
# include <map>
# include <cassert>
# include "../../thread/policy.h"


namespace Yuni
{
namespace Private
{
namespace EventImpl
{

	// Forward declaration
	template<class BindT> class PredicateRemoveObject;
	template<class BindT> class PredicateRemoveObserverBase;
	template<class BindT> class PredicateRemoveWithoutChecks;

} // namespace EventImpl
} // namespace Private
} // namespace Yuni


namespace Yuni
{

	// Forward declarations
	template<class P> class Event;
	class IEvent;
	class IEventObserverBase;
	template<class Derived, template<class> class TP> class IEventObserver;



	/*!
	** \brief
	** \ingroup Events
	*/
	class IEvent
	{
	public:
		//! IEvent list
		typedef std::map<IEvent*, unsigned int> Map;

	public:
		IEvent() {}
		virtual ~IEvent() {}

	protected:
		/*!
		** \brief Unregister an observer
		**
		** This method will remove all delegates linked with the pointer to object.
		** This method should be called by any observer being destroyed.
		**
		** \param pointer Pointer-to-object (can be null)
		*/
		virtual void unregisterObserver(const IEventObserverBase* pointer) = 0;

		// Friend
		template<class Derived, template<class> class TP> friend class IEventObserver;

	}; // class IEvent





	class IEventObserverBase
	{
	public:
		IEventObserverBase() {}
		virtual ~IEventObserverBase() {}

	protected:
		/*!
		** \brief Increment a reference count for a given bound event
		**
		** /param evt An event
		*/
		virtual void boundEventIncrementReference(IEvent* evt) const = 0;

		/*!
		** \brief Decrementing a reference ocunt for a given bound event
		**
		** If the reference count reaches 0, it will be removed from the table.
		** /param evt An event
		*/
		virtual void boundEventDecrementReference(IEvent* evt) const = 0;

		/*!
		** \brief Remove an boundEvent from the table
		**
		** This method is called when the boundEvent is being destroyed.
		** /param evt An event
		*/
		virtual void boundEventRemoveFromTable(IEvent* evt) const = 0;

		// Friends
		template<class P> friend class Event;
		template<class BindT> friend class Private::EventImpl::PredicateRemoveObject;
		template<class BindT> friend class Private::EventImpl::PredicateRemoveObserverBase;

	}; // class IEventObserverBase




	template<class Derived, template<class> class TP = Policy::ObjectLevelLockable>
	class IEventObserver
		: public IEventObserverBase, public TP<IEventObserver<Derived,TP> >
	{
	public:
		//! Type of the Event observer
		typedef IEventObserver<Derived,TP> IEventObserverType;
		//! The Threading policy
		typedef TP<IEventObserverType> ThreadingPolicy;

	public:
		//! \name Constructor & Destructor
		//@{
		IEventObserver();
		/*!
		** \brief Destructor
		*/
		virtual ~IEventObserver();
		//@}

	protected:
		/*!
		** \brief Destroy all existing and future bound events
		*/
		void destroyBoundEvents();

	private:
		// \see IEventObserverBase::boundEventIncrementReference()
		virtual void boundEventIncrementReference(IEvent* evt) const;

		// \see IEventObserverBase::boundEventDecrementReference()
		virtual void boundEventDecrementReference(IEvent* evt) const;

		// \see IEventObserverBase::boundEventRemoveFromTable()
		virtual void boundEventRemoveFromTable(IEvent* evt) const;

	private:
		/*!
		** \brief List of attached boundEvents to the observer
		**
		** This class will accept any action from any bound event as long as
		** this pointer remains valid (not null). It is important to set this
		** variable to NULL when the class is being destroyed to avoid race
		** conditions with SMP processors.
		**
		** \internal The keyword 'mutable' is needed because we may want to bind
		**    an event to a const object (with a const method)
		*/
		mutable IEvent::Map* pBoundEventTable;

	}; // class IEventObserver





} // namespace Yuni

# include "interfaces.hxx"

#endif // __YUNI_CORE_EVENT_INTERFACES_H__

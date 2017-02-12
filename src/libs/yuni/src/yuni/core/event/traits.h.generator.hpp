#ifndef __YUNI_CORE_EVENT_TRAITS_H__
# define __YUNI_CORE_EVENT_TRAITS_H__

<%
require File.dirname(__FILE__) + '/../../../tools/generators/commons.rb'
generator = Generator.new()
%>
<%=generator.thisHeaderHasBeenGenerated("traits.h.generator.hpp")%>

# include "../slist/slist.h"



namespace Yuni
{
namespace Private
{
namespace EventImpl
{

	template<int N, class BindT> class WithNArguments;


	template<typename T>
	class EmptyPredicate
	{
	public:
		typedef void ResultType;
	public:
		inline void operator () (T) {}
		static void result() {}
	};


	template<class BindT>
	class PredicateRemoveObject
	{
	public:
		PredicateRemoveObject(const void* object)
			:pObject(object)
		{}

		bool operator == (const BindT& rhs) const
		{
			return  (pObject == (const void*)rhs.object());
		}
	private:
		const void* pObject;
	};


	template<class BindT>
	class PredicateRemoveObserverBase
	{
	public:
		PredicateRemoveObserverBase(IEvent* event, const IEventObserverBase* object)
			:pEvent(event), pObject(object)
		{}

		bool operator == (const BindT& rhs) const
		{
			if (pObject == rhs.observerBaseObject())
			{
				pObject->boundEventDecrementReference(pEvent);
				return true;
			}
			return false;
		}
	private:
		IEvent* pEvent;
		const IEventObserverBase* pObject;
	};


	template<class BindT>
	class PredicateRemoveWithoutChecks
	{
	public:
		PredicateRemoveWithoutChecks(const IEventObserverBase* object)
			:pObject(object)
		{}

		inline bool operator == (const BindT& rhs) const
		{
			return (rhs.isDescendantOf(pObject));
		}
	private:
		const IEventObserverBase* pObject;
	};



<% (0..generator.argumentCount).each do |i| %>
	template<class BindT>
	class WithNArguments<<%=i%>, BindT> : public Policy::ObjectLevelLockable<WithNArguments<<%=i%>,BindT> >
	{
	public:
		//! The Threading Policy
		typedef Policy::ObjectLevelLockable<WithNArguments<<%=i%>,BindT> > ThreadingPolicy;
		//! Bind
		typedef BindT BindType;
		//! The Return type
		typedef typename BindType::ReturnType R;<% (0..i-1).each do |j| %>
		//! Type of the argument <%=j%>
		typedef typename BindType::template Argument<<%=j%>>::Type A<%=j%>;<% end %>

	public:
		//! \name Constructors
		//@{
		//! Default constructor
		WithNArguments()
			:pEmpty(true)
		{}
		//! Copy constructor
		WithNArguments(const WithNArguments& rhs)
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			pEmpty = rhs.pEmpty;
			pBindList = rhs.pBindList;
		}
		//@}

		//! \name Invoke
		//@{
		/*!
		** \brief Invoke the delegate
		*/
		void invoke(<%=generator.variableList(i)%>) const
		{
			if (!pEmpty)
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				const typename BindList::const_iterator end = pBindList.end();
				for (typename BindList::const_iterator i = pBindList.begin(); i != end; ++i)
					(*i).invoke(<%=generator.list(i,'a')%>);
			}
		}

		template<template<class> class PredicateT>
		typename PredicateT<R>::ResultType invoke(<%=generator.variableList(i)%>) const
		{
			PredicateT<R> predicate;
			if (!pEmpty)
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				const typename BindList::const_iterator end = pBindList.end();
				for (typename BindList::const_iterator i = pBindList.begin(); i != end; ++i)
					predicate((*i).invoke(<%=generator.list(i,'a')%>));
			}
			return predicate.result();
		}

		template<template<class> class PredicateT>
		typename PredicateT<R>::ResultType invoke(PredicateT<R>& predicate<%=generator.variableList(i,"A","a", ", ")%>) const
		{
			if (!pEmpty)
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				const typename BindList::const_iterator end = pBindList.end();
				for (typename BindList::const_iterator i = pBindList.begin(); i != end; ++i)
					predicate((*i).invoke(<%=generator.list(i,'a')%>));
			}
			return predicate.result();
		}


		template<class EventT> void assign(EventT& rhs)
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			typename ThreadingPolicy::MutexLocker lockerRHS(rhs);
			pBindList = rhs.pBindList;
			pEmpty = pBindList.empty();
		}

		/*!
		** \brief Invoke the delegate
		*/
		void operator () (<%=generator.variableList(i)%>) const
		{
			if (!pEmpty)
			{
				typename ThreadingPolicy::MutexLocker locker(*this);
				const typename BindList::const_iterator end = pBindList.end();
				for (typename BindList::const_iterator i = pBindList.begin(); i != end; ++i)
					(*i).invoke(<%=generator.list(i,'a')%>);
			}
		}
		//@}

	protected:
		//! Binding list (type)
		typedef LinkedList<BindType> BindList;
		//! A flag to know if the event is empty or not
		// This value must only set when the mutex is locked
		volatile bool pEmpty;
		//! Binding list
		BindList pBindList;

		// friend !
		template<class P> friend class Event;

	}; // class WithNArguments


<% end %>




} // namespace EventImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_EVENT_TRAITS_H__

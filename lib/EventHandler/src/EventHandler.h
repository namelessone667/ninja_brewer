#ifndef __EVENTHANDLER_H_
#define __EVENTHANDLER_H_

#include <map>
#include <unordered_set>
#include <memory>

	class CEventSource;	//Forward declaration

	class CEventHandlerArgs
	{
	public:
		CEventHandlerArgs() {}
		virtual ~CEventHandlerArgs() { }
	};

	/// <Summary>
	/// For Class-based Event Subscriptions, a class can receive
	/// events only if it is derived from CEventReceiver.
	/// <para> See CEvent for sample usage.</para>
	/// </Summary>
	class CEventReceiver
	{
	protected:
		// Protected constructor. CEventReceiver objects cannot be created directly.
		// This Class should be inherited.
		CEventReceiver() {}

		// Protected destructor.
		// No need to declare as virtual because delete can't be applied on CEventReceiver pointers.
		~CEventReceiver() { }
	};

	/// <Summary>
	/// Class for Event Subscription Function Objects.
	/// </Summary>
	class IEventFunctor
	{
	public:
		virtual ~IEventFunctor() { }
		virtual void operator()(CEventReceiver* pReceiver, const CEventSource* pSender, CEventHandlerArgs* pArgs) = 0;
	};

	/// <Summary>
	/// A Receiver can Subscribe for an Event only once.
	/// If a Receiver requests a second subscription for the same Event,
	/// its previous subscription would be replaced with the new subscription.
	///
	/// A Receiver can subscribe for as many different events as it wants.
	/// But for each, only once.
	///
	/// And an Event can have as many Receivers as it likes.
	/// </Summary>
	/// <example>Following Snippet demonstrates sample usage.
	/// <code>
	/// class MySender : public CEventSource
	/// {
	///		public:
	///		CEvent m_Event;
	///		...
	///		void MyFunction()
	///		{
	///			...
	///			RaiseEvent(&m_Event, &CEventHandlerArgs());	// Invokes the Event
	///			...
	///		}
	///		...
	///	};
	///	class MyReceiver: public CEventReceiver
	///	{
	///		MyClass	MySenderObject;
	///	public:
	///		MyReceiver()
	///		{
	///			// Subscribe for the Event
	///			MySenderObject.m_Event.Subscribe(this, & MyReceiver::EventReceptionFunc);
	///		}
	///		void EventReceptionFunc(const CEventSource* pSrc, CEventHandlerArgs* pArgs)
	///		{
	///			// Code for Handling the Event
	///		}
	///	};
	/// </code>
	/// </example>
	class CEvent
	{
		/// <Summary>
		/// Implements IEventFunctor for Handling Class-based Subscriptions.
		/// </Summary>
		template<typename TClass>
		class CClassFunctor : public IEventFunctor
		{
			typedef void (TClass::*FuncType)(const CEventSource* pSender, CEventHandlerArgs* pArgs);

			FuncType m_pFunc;

			CClassFunctor(FuncType pFunc) : m_pFunc(pFunc) { }

			inline void operator()(CEventReceiver* pReceiver, const CEventSource* pSender, CEventHandlerArgs* pArgs)
			{
				(((TClass*)pReceiver)->*m_pFunc)(pSender, pArgs);
			}

			friend class CEvent; // Only CEvent Objects can use the CClassFunctor Objects
		};

		typedef std::unique_ptr<IEventFunctor> IEventFunctorPtr;
		typedef std::map<CEventReceiver*, IEventFunctorPtr> CLASS_HANDLER_MAP;
		typedef void (*STATICHANDLER)(const CEventSource*, CEventHandlerArgs*);
		typedef std::unordered_set<STATICHANDLER> STATIC_HANDLER_LIST;

		CLASS_HANDLER_MAP	m_ClassSubscribers;
		STATIC_HANDLER_LIST	m_StaticSubscribers;

		/// <Summary>
		/// Events can be Invoked only through CEventSource::RaiseEvent()
		/// </Summary>
		void Invoke(const CEventSource* pSender, CEventHandlerArgs* pArgs)
		{

      for (auto it = m_ClassSubscribers.begin(); it != m_ClassSubscribers.end(); ++it)
      {

        (*it->second)(it->first, pSender, pArgs);

      }

      for (auto it = m_StaticSubscribers.begin(); it != m_StaticSubscribers.end(); ++it)
      {

        (*it)(pSender, pArgs);

      }
		}

		friend class CEventSource;

		CEvent& operator = (const CEvent& );	// Prohibit Assignment

		CEvent(const CEvent& );		// Prohibit Copy Constructor

	public:
		inline CEvent() { }

		inline ~CEvent() {	}

		/// <Summary>
		/// Creates an IEventFunctor object for the given function and
		/// subcribes it to be called upon the given object whenever the event
		/// is raised.
		/// Duplicates or Multiple Subscriptions are not allowed.
		/// If the Receiver object already has a subscription for this event,
		/// the old subscription is removed before adding the new subcription.
		/// </Summary>
		template<typename TClass>
		inline void Subscribe(TClass* pReceiver, void (TClass::*lpfnHandler)(const CEventSource*, CEventHandlerArgs* ))
		{
			if(pReceiver == NULL)
        return;

			IEventFunctorPtr functorptr = IEventFunctorPtr(new CClassFunctor<TClass>(lpfnHandler));
			Subscribe(pReceiver, functorptr);
		}

		/// <Summary>
		/// Takes ownership of the supplied IEventFunctor pointer.
		/// Assumes that it is allocated with new, and calls delete
		/// automatically upon it when going out of scope.
		/// Duplicate or Multiple Subscriptions are not allowed.
		/// If the Receiver object already has a subscription for this event,
		/// the old subscription is removed before adding the new subcription.
		/// </Summary>
		template<typename TReceiverClass>
		inline void Subscribe(TReceiverClass* pReceiver, std::unique_ptr<IEventFunctor>& spFunctor)
		{
			if(pReceiver == NULL || spFunctor == NULL)
        return;

			m_ClassSubscribers[pReceiver] = std::move(spFunctor);
		}

		/// <Summary>
		/// Subscribes the given Function to be called whenever the event is raised.
		/// Duplicates are not Allowed.
		/// If the given Function is already a subscriber, it would not be added again.
		/// </Summary>
		inline void Subscribe(void (*lpfnHandler)(const CEventSource*, CEventHandlerArgs*))
		{
			if(NULL == lpfnHandler)
        return;

      if(m_StaticSubscribers.find(lpfnHandler) != m_StaticSubscribers.end())
			   m_StaticSubscribers.insert( lpfnHandler );
		}

		/// <Summary>
		/// UnSubscribes the given Function from being called.
		/// Nothing Happens if the given Function is not a Subscriber.
		/// </Summary>
		inline void UnSubscribe(void (*lpfnHandler)(const CEventSource*, CEventHandlerArgs*))
		{
				m_StaticSubscribers.erase(lpfnHandler);
		}

		/// <Summary>
		/// UnSubscribes the pReceiver from the Subscription.
		/// Nothing Happens if the supplied pReceiver object is not a Subscriber.
		/// </Summary>
		template<typename TReceiverClass>
		inline void UnSubscribe(TReceiverClass* pReceiver)
		{
			m_ClassSubscribers.erase(pReceiver);
		}

		/// <Summary>
		/// UnSubscribes all receivers. SubscriberCount() would become Zero.
		/// </Summary>
		inline void UnSubscribeAll()
		{
			// Remove Static Subscribers
			m_StaticSubscribers.clear();
			// Remove Class-based Subscribers
			m_ClassSubscribers.clear();
		}

		/// <Summary>
		/// Gives the number of active Subcriptions for this event.
		/// It includes the Class-based Subscriptions as well the Function-based ones.
		/// </Summary>
		inline size_t SubscriberCount() const
		{
			return m_ClassSubscribers.size() + m_StaticSubscribers.size();
		}
	};

	/// <Summary>
	/// Only CEventSource derived classes can raise events.
	/// <para>See CEvent for sample usage.</para>
	/// </Summary>
	class CEventSource
	{
	protected:
		// CEventSource objects cannot be created directly due to the protected constructor.
		// This class can Only be Inherited.
		CEventSource() {}

		// Protected destructor.
		// No need to declare as virtual because delete can't be applied on CEventSource pointers.
		~CEventSource() { }

		/// <Summary>
		/// Invokes the Event.
		/// </Summary>
		inline void RaiseEvent(CEvent* pEvent, CEventHandlerArgs* pArgs)
		{
			if(pEvent == NULL)
        return;

			pEvent->Invoke(this, pArgs);
		}
	};

	/// <Summary>
	/// An event that can be invoked by itself
	/// </Summary>
	class CInvokableEvent : protected CEventSource, public CEvent
	{
	public:
		inline void RaiseEvent(CEventHandlerArgs* pArgs)
		{
			CEventSource::RaiseEvent(this, pArgs);
		}
	};

#endif	// __EVENTHANDLER_H

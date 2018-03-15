#ifndef Property_h
#define Property_h

#include "EventHandler.h"

template<typename T>
	class CValueChangedEventArgs: public CEventHandlerArgs
	{
	protected:
		T		m_NewValue;				// Indicates the New Value, if used

	public:
		template<typename NEWVALTYPE>
		CValueChangedEventArgs(const NEWVALTYPE& NewVal)
			:	m_NewValue(NewVal)
		{
		}

		/// <Summary>Gives the Value after the change, subjected to
		/// IsNewValueValid() being True.</Summary>
		inline const T&	NewValue() const	{	return m_NewValue;	}
	};

template<typename T>
	class Property : public CEventSource
  {
    protected:
      T m_Data;		// Create the Object for Predefined Types

    public:
      typedef CValueChangedEventArgs<T> ChangedEvArgs;

      CEvent	ValueChanged;		///< Event Raised after the value changed

			inline Property() {};

			inline Property(const T value) { m_Data = value;};

			inline Property(const Property& value) { m_Data = value.Get();};

      inline void Set(const T newValue)
      {
				if(m_Data != newValue)
				{
        	m_Data = newValue;
        	ChangedEvArgs evArgs(m_Data);
					RaiseEvent(&ValueChanged, &evArgs);
				}
      }

      inline const T& Get() const {return m_Data;}

			inline Property& operator=(const T& other)
			{
				Set(other);
				return *this;
			}

			inline Property& operator=(const Property& other)
			{
				Set(other.Get());
				return *this;
			}

			inline operator T() const { return m_Data; }
  };

#endif

#ifndef ReadOnlyProperty_h_
#define ReadOnlyProperty_h_

#include "Property.h"

template<typename T>
	class ReadOnlyProperty
  {
  public:
    ReadOnlyProperty(Property<T>& encapsulatedProperty) :
      _encapsulatedProperty(encapsulatedProperty)
    {

    }

    const T& Get() const
    {
      return _encapsulatedProperty.Get();
    }

    CEvent& ValueChanged()
    {
      return _encapsulatedProperty.ValueChanged;
    }
  private:
    Property<T>& _encapsulatedProperty;
  };

#endif

#ifndef PIDProxy_h
#define PIDProxy_h

#include "PID_v1.h"
#include "Property.h"

class PIDProxy : public PID, public CEventReceiver
{
//TODO: Bind tunning parameters, PID mode

public:
  PIDProxy (Property<double>& Input, Property<double>& Output, Property<double>& Setpoint, double Kp, double Ki, double Kd, int ControllerDirection)
    : PID (&m_input, &m_output, &m_setpoint, Kp, Ki, Kd, ControllerDirection)
  {
    m_input = Input;
    m_output = Output;
    m_setpoint = Setpoint;

    Input.ValueChanged.Subscribe(this, &PIDProxy::HandleInputChanged);
    Setpoint.ValueChanged.Subscribe(this, &PIDProxy::HandleSetpointChanged);
  }

  void HandleInputChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
		m_input = ((CValueChangedEventArgs<double>*)EvArgs)->NewValue();
	}

  void HandleSetpointChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
		m_setpoint = ((CValueChangedEventArgs<double>*)EvArgs)->NewValue();
	}

  const double GetOutput()
  {
    return m_output;
  }

private:
  double m_input;              // * Pointers to the Input, Output, and Setpoint variables
  double m_output;             //   This creates a hard link between the variables and the
  double m_setpoint;           //   PID, freeing the user from having to constantly tell us
};

#endif

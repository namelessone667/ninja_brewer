#ifndef PIDProxy_h
#define PIDProxy_h

#include "PID_v1.h"
#include "Property.h"

class PIDProxy : public PID, public CEventReceiver
{
  //TODO: Bind PID Mode
  //TODO: create 2 new control variables - integrator error multiplier positive / negative
  //TODO: implement anti integrator windup - backcalculation

public:
  PIDProxy (Property<double>& input, Property<double>& output, Property<double>& setpoint, Property<double>& Kp, Property<double>& Ki, Property<double>& Kd, int ControllerDirection)
    : PID (&m_input, &m_output, &m_setpoint, Kp, Ki, Kd, ControllerDirection)
  {
    m_input = input;
    m_output = output;
    m_setpoint = setpoint;

    Output.Set(m_output);

    input.ValueChanged.Subscribe(this, &PIDProxy::HandleInputChanged);
    setpoint.ValueChanged.Subscribe(this, &PIDProxy::HandleSetpointChanged);
    Output.ValueChanged.Subscribe(this, &PIDProxy::HandleOutputChanged);
    Kp.ValueChanged.Subscribe(this, &PIDProxy::HandleKpChanged);
    Ki.ValueChanged.Subscribe(this, &PIDProxy::HandleKiChanged);
    Kd.ValueChanged.Subscribe(this, &PIDProxy::HandleKdChanged);
  }

  bool Compute()
  {
    if(PID::Compute())
    {
      inCompute = true;
      Output.Set(m_output);
      inCompute = false;
      return true;
    }
    return false;
  }

  Property<double> Output;
  bool inCompute = false;

private:
  void HandleInputChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
		m_input = ((CValueChangedEventArgs<double>*)EvArgs)->NewValue();
	}

  void HandleSetpointChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
		m_setpoint = ((CValueChangedEventArgs<double>*)EvArgs)->NewValue();
	}

  void HandleOutputChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
    if(!inCompute)
		  m_output = ((CValueChangedEventArgs<double>*)EvArgs)->NewValue();
	}

  void HandleKpChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
    SetTunings(((CValueChangedEventArgs<double>*)EvArgs)->NewValue(), GetKi(), GetKd());
	}

  void HandleKiChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
		SetTunings(GetKp(), ((CValueChangedEventArgs<double>*)EvArgs)->NewValue(), GetKd());
	}

  void HandleKdChanged(const CEventSource* EvSrc,CEventHandlerArgs* EvArgs)
	{
		SetTunings(GetKp(), GetKi(), ((CValueChangedEventArgs<double>*)EvArgs)->NewValue());
	}

  double m_input;              // * Pointers to the Input, Output, and Setpoint variables
  double m_output;             //   This creates a hard link between the variables and the
  double m_setpoint;           //   PID, freeing the user from having to constantly tell us

};

#endif

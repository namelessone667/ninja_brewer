#include "ParticlePublisher.h"
#include "application.h"

void ParticlePublisher::init(const Model &model)
{
    Particle.connect();

    Particle.variable("beerTemp", model._appState.beerTemp);
    Particle.variable("fridgeTemp", model._appState.fridgeTemp);
    Particle.variable("pidKp", model._appConfig.pid_Kp);
    Particle.variable("pidKi", model._appConfig.pid_Ki);
    Particle.variable("pidKd", model._appConfig.pid_Kd);
    Particle.variable("heatPidKp", model._appConfig.heatpid_Kp);
    Particle.variable("heatPidKi", model._appConfig.heatpid_Ki);
    Particle.variable("heatPidKd", model._appConfig.heatpid_Kd);

    //Particle.function();

    /*
    Particle.function("setPIDAuto", setPIDModeAuto);
    Particle.function("setPIDManual", setPIDModeManual);
    Particle.function("setHPIDMan", setHeatPIDManual);
    Particle.function("setHPIDAuto", setHeatPIDAuto);
    Particle.function("reboot", reboot);
    Particle.function("setPIDKi", setPIDKi);
    Particle.function("setPIDKp", setPIDKp);
    Particle.function("setPIDKd", setPIDKd);
    Particle.function("setHeatPIDKi", setHeatPIDKi);
    Particle.function("setHeatPIDKp", setHeatPIDKp);
    Particle.function("setHeatPIDKd", setHeatPIDKd);
    Particle.function("startControl", startControl);
    Particle.function("stopControl", stopControl);
    Particle.function("saveState", saveState);
    Particle.function("getDevices", getOneWireDevices);
    Particle.function("reinitLCD", reinitLCD);
    */
}

void ParticlePublisher::publish(const Model &model)
{

}

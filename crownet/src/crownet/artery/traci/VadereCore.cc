/*
 * RoVerCore.cc
 *
 *  Created on: Aug 6, 2020
 *      Author: sts
 */
#include "VadereCore.h"

#include <inet/common/ModuleAccess.h>
#include <omnetpp/checkandcast.h>
#include <traci/SubscriptionManager.h>

#include "crownet/artery/traci/VadereLauchner.h"
#include "crownet/artery/traci/VadereSubscriptionManager.h"

using namespace traci;
using namespace omnetpp;

namespace {
const simsignal_t connectedSignal = cComponent::registerSignal("traci.connected");
const simsignal_t initSignal = cComponent::registerSignal("traci.init");
const simsignal_t stepSignal = cComponent::registerSignal("traci.step");
const simsignal_t closeSignal = cComponent::registerSignal("traci.close");
}  // namespace

namespace crownet {

Define_Module(VadereCore);

void VadereCore::handleMessage(omnetpp::cMessage* msg) {
  if (msg == m_updateEvent) {
    // mobility provider is ahead dt = updateInterval
    // needed to interpolate between NOW(=simTime()) and NOW+updateInterval
    simtime_t targetTime = simTime() + m_updateInterval;
    m_traci->simulationStep(targetTime.dbl());
    if (m_subscriptions) {
      m_subscriptions->step();
    }
    emit(stepSignal, simTime());

    if (!m_stopping) {
      scheduleAt(simTime() + m_updateInterval, m_updateEvent);
    }
  } else if (msg == m_connectEvent) {
      ServerEndpoint endpoint = m_launcher->launch();
      try{
          m_traci->connect(endpoint);
      } catch (tcpip::SocketException e) {
          throw omnetpp::cRuntimeError("Cannot connect to Vadere server using <<%s:%s>>. "
                  "Is server or container started or check hostname and port configuration?",
                  endpoint.hostname.c_str(), std::to_string(endpoint.port).c_str());
      }
      checkVersion();
      syncTime();
      // pre subscribe
      auto traciLauchner = omnetpp::check_and_cast<VadereLauchner*>(m_launcher);
      traciLauchner->initializeServer(m_traci);
      emit(connectedSignal, simTime());
      // send initSignal to setup subscriptions
      emit(initSignal, simTime());
      m_updateInterval = Time{m_traci->simulation.getDeltaT()};
      scheduleAt(simTime() + m_updateInterval, m_updateEvent);
  }
}

std::shared_ptr<TraCiForwarder> VadereCore::getTraCiForwarder(){
    return  std::dynamic_pointer_cast<VadereApi>(this->m_traci);
}


std::shared_ptr<VadereApi> VadereCore::getVadereApi(){
    return std::dynamic_pointer_cast<VadereApi>(m_traci);
}

} /* namespace crownet */

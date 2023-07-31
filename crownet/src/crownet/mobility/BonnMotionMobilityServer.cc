/*
 * BonnMotionMobilityServer.cc
 *
 *  Created on: Jun 27, 2022
 *      Author: vm-sts
 */

#include "BonnMotionMobilityServer.h"
#include <inet/common/ModuleAccess.h>
#include "crownet/artery/traci/TraCiNodeVisitorAcceptor.h"
#include "inet/common/scenario/ScenarioManager.h"
#include "inet/mobility/single/BonnMotionFileCache.h"
#include "crownet/mobility/BonnMotionMobilityClient.h"
#include "crownet/common/GlobalDensityMap.h"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <fstream>
#include <iostream>

#include <list>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>

#include "inet/common/INETDefs.h"



namespace crownet {

void BonnMotionServerFile::loadFile(const char *filename, bool is3D){
    std::string fname(filename);
    std::stringstream inStr;
    if (fname.compare(fname.size()-3, 3, ".gz") == 0){
        //gz
        std::ifstream file(fname.c_str(), std::ios_base::in | std::ios_base::binary);
        boost::iostreams::filtering_streambuf<boost::iostreams::input> gzIn;
        gzIn.push(boost::iostreams::gzip_decompressor());
        gzIn.push(file);
        boost::iostreams::copy(gzIn, inStr);
    } else {
        std::ifstream file(fname.c_str(), std::ios::in);
        if (file.fail())
            throw cRuntimeError("Cannot open file '%s'", filename);
        boost::iostreams::copy(file, inStr);
    }

    std::string line;
    int lineCount = 0;
    int minSize = is3D ? 4 : 3;
    while (std::getline(inStr, line)) {
        if(line.at(0) == '#'){
            continue; // ignore comets
        }
        lines.push_back(BonnMotionFile::Line());
        BonnMotionFile::Line& vec = lines.back();

        std::stringstream linestream(line);
        double d;
        while (linestream >> d)
            vec.push_back(d);




        if (((int)vec.size() % minSize) != 0 ){
            throw cRuntimeError("Expected multiple of %i elements in row got %i for line %i",
                        minSize, (int)vec.size(), lineCount);
        }
        if ((int)vec.size() == minSize){
            throw cRuntimeError("Expected at least %i elements but got %i for line %i in %s mode. (Need at least 2 points for speed interpolation.)",
                    2*minSize, (int)vec.size(), lineCount, is3D ? "3D" : "2D");
        }

        timeLine.push_back(std::make_pair(lineCount, simtime_t(vec[0])));
        ++lineCount;
    }

    // sort timeLine based on time step **and** keep order of lines with the same time step!
    std::stable_sort(
            timeLine.begin(), timeLine.end(),
            [](const BmTimedLineIndex& left, const BmTimedLineIndex& right){return left.second < right.second;}
    );

}

bool BonnMotionServerFile::hasTraceForTime(const simtime_t time) const{

    return nextTimeLineIndex < timeLine.size() && timeLine[nextTimeLineIndex].second <= time.dbl();
}

const BmTimedLineIndex BonnMotionServerFile::getNextTimeLineIndex(const simtime_t time){
    if (!hasTraceForTime(time)){
        throw cRuntimeError("no trace for given time: %s", time.str().c_str());
    }
    auto ret = timeLine[nextTimeLineIndex];
    ++nextTimeLineIndex;
    return ret;
}

bool BonnMotionServerFile::hasNextTimeLineIndex() const {
    return nextTimeLineIndex < timeLine.size();
}

const BmTimedLineIndex BonnMotionServerFile::peekAtNextTimeLineIndex(){
    if (!hasNextTimeLineIndex()){
            throw cRuntimeError("No traces left");
        }
    return timeLine[nextTimeLineIndex];
}



Define_Module(BonnMotionMobilityServer);

const omnetpp::simsignal_t BonnMotionMobilityServer::bonnMotionTargetReached =
        cComponent::registerSignal("bonnMotion.targetReached");

BonnMotionMobilityServer::BonnMotionMobilityServer() {
    // TODO Auto-generated constructor stub

}

BonnMotionMobilityServer::~BonnMotionMobilityServer() {
    if (creationTimer){
        cancelAndDelete(creationTimer);
    }
}

void BonnMotionMobilityServer::receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) {
  Enter_Method_Silent();
  if (signalID == bonnMotionTargetReached) {
      cMessage* deleteMsg = new cMessage("");
      deleteMsg->setKind(DELETE_MSG);
      nodesToDelete.push_back((int)d);
      scheduleAt(simTime(), deleteMsg); // now
  } else {
      // err
  }
}

void BonnMotionMobilityServer::initialize(int stage)
{
    cSimpleModule::initialize(stage);

    EV_TRACE << "initializing BonnMotionMobilityServer stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        getSystemModule()->subscribe(bonnMotionTargetReached, this);
        node_type = cModuleType::find(par("moduleType"));
        moduleVector = par("vectorNode").stdstringValue();
        is3D = par("is3D").boolValue();
        bmFile.loadFile(par("traceFile").stringValue(), is3D);
        m_mobility = par("mobilityModulePath").stdstringValue();
        creationTimer = new cMessage("BonnMotionCreationTimer");

        scheduleNextCreationEvent();

        emit(GlobalDensityMap::registerNodeAcceptor, this);
    }
}

void BonnMotionMobilityServer::handleMessage(cMessage *msg){
    if (msg == creationTimer){
        simtime_t now = simTime();

        // create all nodes with current creation time
        while(bmFile.hasTraceForTime(now)){
            auto timeLineIndex = bmFile.getNextTimeLineIndex(simTime());

            NodeInitializer init = [this, &timeLineIndex](cModule* node) {
                auto mobily_m = node->findModuleByPath(m_mobility.c_str());
                if(!mobily_m){
                    throw cRuntimeError("No module found at %s relative to %s",
                                        m_mobility.c_str(),
                                        node->getFullPath().c_str());

                }
                auto mobily = dynamic_cast<BonnMotionMobilityClient*>(mobily_m);
                if(!mobily){
                    throw cRuntimeError("Module %s has no or wrong mobility module. Expected BonnMotionMobilityClient",
                            node->getFullPath().c_str());

                }
                mobily->initTrace(bmFile.getLine(timeLineIndex.first), is3D, timeLineIndex.first);
            };


            addNodeModule(timeLineIndex.first, node_type, init);
        }

        scheduleNextCreationEvent();
    } else if (msg->isSelfMessage() && msg->getKind() == DELETE_MSG){
        // trigger to delete nodes.
        // This might delete multiple nodes if multiple nodes
        // are deleted at the same time. Subsequent delete msg
        // will just be deleted.
        for (const auto id: nodesToDelete){
            removeNodeModule(id);
        }
        nodesToDelete.clear();
        cancelAndDelete(msg);
    } else {
        throw cRuntimeError("Wrong message received %s", msg->getName());
    }

}


omnetpp::cModule* BonnMotionMobilityServer::createModule(omnetpp::cModuleType* type){
    cModule* parentModule = getSystemModule();

    if (!parentModule->hasSubmoduleVector(moduleVector.c_str())) {
        parentModule->addSubmoduleVector(moduleVector.c_str(), moduleVectorIndex + 1);
    }
    else
        parentModule->setSubmoduleVectorSize(moduleVector.c_str(), moduleVectorIndex + 1);
    cModule* module = type->create(moduleVector.c_str(), parentModule, moduleVectorIndex);

    ++moduleVectorIndex;
    return module;

}
omnetpp::cModule* BonnMotionMobilityServer::addNodeModule(const int bmLine,
                                        omnetpp::cModuleType* type,
                                        NodeInitializer& init){
    cModule* module = createModule(type);
    module->finalizeParameters();
    module->buildInside();
    nodeMap[bmLine] = module;
    init(module);
    module->scheduleStart(simTime());

    inet::cPreModuleInitNotification pre;
    pre.module = module;
    emit(POST_MODEL_CHANGE, &pre);
    module->callInitialize();
    inet::cPostModuleInitNotification post;
    post.module = module;
    emit(POST_MODEL_CHANGE, &post);

    return module;
}

void BonnMotionMobilityServer::removeNodeModule(const int bmLine){
    Enter_Method_Silent();
    cModule* module = getNodeModule(bmLine);
    if (module) {
      module->callFinish();
      module->deleteModule();
      nodeMap.erase(bmLine);
    } else {
      EV_DEBUG << "Node with BonnMotion trace " << bmLine << " does not exist, no removal\n";
    }
}

cModule* BonnMotionMobilityServer::getNodeModule(const int bmLine) {
  auto found = nodeMap.find(bmLine);
  return found != nodeMap.end() ? found->second : nullptr;
}


void BonnMotionMobilityServer::scheduleNextCreationEvent(){
    cancelEvent(creationTimer);
    if (bmFile.hasNextTimeLineIndex()){
        auto timeLineIndex = bmFile.peekAtNextTimeLineIndex();
        scheduleAt(timeLineIndex.second, creationTimer);
    }
}

void BonnMotionMobilityServer::acceptTraciVisitor(traci::ITraciNodeVisitor* visitor){
    for(const auto& entry: nodeMap){
        visitor->visitNode("", entry.second); //id not used
    }
}




} /* namespace crownet */

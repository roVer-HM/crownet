/*
 * BonnMotionMobilityServer.h
 *
 *  Created on: Jun 27, 2022
 *      Author: vm-sts
 */

#pragma once

#include <omnetpp.h>
#include <omnetpp/clistener.h>
#include <omnetpp/csimplemodule.h>
#include <functional>
#include "inet/mobility/single/BonnMotionFileCache.h"
#include "crownet/artery/traci/TraCiNodeVisitorAcceptor.h"


using namespace inet;
using namespace omnetpp;

namespace crownet {

using BmTimedLineIndex = std::pair<int, simtime_t>;


class BonnMotionServerFile : public BonnMotionFile{

public:
    BonnMotionServerFile(): BonnMotionFile() {}
    std::vector<BmTimedLineIndex> getTimeLine() const;
    void loadFile(const char *filename, bool is3D = false);

    bool hasTraceForTime(const simtime_t time) const;
    const BmTimedLineIndex getNextTimeLineIndex(const simtime_t time);

    bool hasNextTimeLineIndex() const;
    const BmTimedLineIndex peekAtNextTimeLineIndex();

protected:
    std::vector<BmTimedLineIndex> timeLine;
    int nextTimeLineIndex = 0;
};


class BonnMotionMobilityServer : public omnetpp::cSimpleModule,
                                 public omnetpp::cListener,
                                 public ITraCiNodeVisitorAcceptor
{
public:
    static const omnetpp::simsignal_t bonnMotionTargetReached;
    static const short int DELETE_MSG = 13;

public:
    BonnMotionMobilityServer();
    virtual ~BonnMotionMobilityServer();

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;

    // cListner
    using omnetpp::cIListener::finish;  // [-Woverloaded-virtual]
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, double d, cObject *details) override;
    // ITraCiNodeVisitorAcceptor
    virtual void acceptTraciVisitor(traci::ITraciNodeVisitor* visitor) override;

protected:
    using NodeInitializer = std::function<void(omnetpp::cModule*)>;

    virtual omnetpp::cModule* createModule(omnetpp::cModuleType* type);
    virtual omnetpp::cModule* addNodeModule(const int bmLine,
                                            omnetpp::cModuleType*,
                                            NodeInitializer&);
    virtual void removeNodeModule(const int bmLine);
    virtual cModule* getNodeModule(const int bmLine);

    virtual void handleMessage(cMessage *msg) override;


    virtual void scheduleNextCreationEvent();



protected:

    cMessage* creationTimer; // trigger creation of new node based on first time
                               // in BonnMotion trace file

    /*
     * Map of created nodes key: line number in traceFile;
     */
    std::map<int, cModule*> nodeMap;
    std::vector<int> nodesToDelete;

    /*
     * node to create for each trace
     */
    omnetpp::cModuleType* node_type;
    std::string moduleVector;
    std::string m_mobility;
    int moduleVectorIndex = 0;
    /*
     * Trace File
     */
    BonnMotionServerFile bmFile;
    bool is3D;
    /*
     * Sorted list of bmLines after creation time.
     * The creationTimer is used to time when the next node
     * should be created.
     */

};

} /* namespace crownet */


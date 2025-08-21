#pragma once

#include "modernize.h"
#include <mainStructs.h>
#include <set>
#include <iostream>




struct EventChain;
struct Circuit;

struct EventSystemFunctionData{
    EventChain* executeNext;
    union {
        struct {
            f32 packetSize;
            u32 currentISPIndex;
            LocalNode* destinationNode;
            LocalNode* sourceNode;
            f32 userIdleTimeAfterRequest;
            bool isMarked;
        }SendVoidPacketFromISPData;
        struct {
            f32 packetSize;
            LocalNode* currentNode;
            LocalNode* destinationNode;
            LocalNode* sourceNode;
            f32 userIdleTimeAfterRequest;

            bool isMarked;
        }SendVoidPacketFromLocalNodeData;
    };
};



using EventFunctionPTR = funcPtr<void(f32, u32, EventSystemFunctionData)>;


namespace eventSystem {

    inline static u32 getNextID() {
        static u32 ID = 0;
        ++ID;
        return ID;
    }

    

    struct Event{
        f32 timeOfExecution;
        u32 eventID;
        EventSystemFunctionData data;
        EventFunctionPTR eventFunction;

        bool operator<(const Event& other) const {
            if (timeOfExecution < other.timeOfExecution) {
                return true;
            }
            else if (other.timeOfExecution < timeOfExecution) {
                return false;
            }
            else {
                return eventID < other.eventID;
            }
        }
    };
    inline std::set<Event> eventsQueue;
    

   
    inline bool isNotEmpty() {
        return eventsQueue.size() != 0;
    }

    inline void runNextEvent() {
        Event e = *eventsQueue.begin();
        eventsQueue.erase(eventsQueue.begin());
        e.eventFunction(e.timeOfExecution,e.eventID,e.data);
    }

    
    inline void addEvent(f32 timeOfExecution, u32 ID, EventSystemFunctionData data, EventFunctionPTR eventFunction) {
        Event tmp = Event {
            .timeOfExecution = timeOfExecution,
            .eventID = ID,
            .data = data,
            .eventFunction = eventFunction
        };
        eventsQueue.insert(tmp);
    }
    
}

struct EventChain {

    struct Event {
        EventSystemFunctionData data;
        EventFunctionPTR eventFunction;

        void addToEventSystem(f32 timeOfExecution) {
            eventSystem::addEvent(timeOfExecution, eventSystem::getNextID(),
                data,
                eventFunction
            );
        }
    };

    u32 nextEventToExecute;
    u32 size;
    Event myEvents[1]; // Hack for variable array size

    
    static void executeNextEventInChain(f32 timeOfExecution, EventChain* executeNext) {


        if (executeNext == nullptr) { // If we don't belong to an event chain then there is no event to execute afterwards
            return;
        }

        if (executeNext->nextEventToExecute < executeNext->size) { //If we have more events to execute
            
            //
            //Add next event to the eventSystem, then increment which is the next event we need to execute
          
            executeNext->myEvents[executeNext->nextEventToExecute].addToEventSystem(timeOfExecution);

            executeNext->nextEventToExecute += 1;
        } 
        else {//If there are no more events to execute, free the memory 
            free(executeNext);
        }

    }

    template<usize N>
    static EventChain* makeEventChain(const Event(&arr)[N]) {

        EventChain* chain;
        chain = (EventChain*)malloc(sizeof(EventChain) + (N - 1) * sizeof(Event));
        assertWithMessage(chain!=nullptr, "Error, allocation failure");

        chain->size = (u32) N;
        chain->nextEventToExecute = 0;

        for (usize i=0; i < N; ++i) {
            chain->myEvents[i] = arr[i];
            chain->myEvents[i].data.executeNext = chain;
        }       
        return chain;
    }

    static EventChain* addEventChains(std::initializer_list<EventChain*> arr) { //TAKES OWNERSHIP OF MEMORY, ignores nullptrs
        // arr.size() gives the number of elements
        u32 N = 0;
        for (EventChain* ptr : arr) {
            if (ptr != nullptr) {
                N += ptr->size;
            }
        }
        assertWithMessage(N != 0, "Error, need at least 1 event");

        EventChain* chain;
        chain = (EventChain*)malloc(sizeof(EventChain) + (N - 1) * sizeof(Event));
        assertWithMessage(chain!=nullptr, "Error, allocation failure");
        chain->size = (u32) N;
        chain->nextEventToExecute = 0;

        u32 currentIndex = 0;
        for (EventChain* ptr : arr) {
            if (ptr != nullptr) {
                for (u32 i = 0; i < ptr->size; ++i) {
                    chain->myEvents[currentIndex] = ptr->myEvents[i];
                    chain->myEvents[currentIndex].data.executeNext = chain;
                    ++currentIndex;
                }
                free(ptr);
            }
        }
        return chain;
    }

    static EventChain* multiplyBy(EventChain* e, u32 multiplier,std::vector<double> dwell_times) { //Takes ownership of memory
        assertWithMessage(multiplier > 0, "Can't multiply event chain with non positive number");
        assertWithMessage(static_cast<uint32_t>(dwell_times.size())==multiplier,"dwell times size does not equal multiplier");
        u32 N = multiplier * e->size;
        assertWithMessage((dwell_times.size())==(N/e->size),"Wrong duell times vector size");
        EventChain* chain;
        //Could optimize with realloc, but malloc is simpler
        chain = (EventChain*)malloc(sizeof(EventChain) + (N - 1) * sizeof(Event));
        assertWithMessage(chain!=nullptr, "Error, allocation failure");
        chain->size = N;
        chain->nextEventToExecute = 0;

        u32 currentIndex = 0;
        u32 dwellIndex=0;
        for (u32 j = 0; j < multiplier; ++j) {
            for (u32 i = 0; i < e->size; ++i) {
                chain->myEvents[currentIndex] = e->myEvents[i];
                chain->myEvents[currentIndex].data.executeNext = chain;
                if(currentIndex%e->size==0)
                {
                    int howManyTimesIsBiggerThanChainSize=currentIndex/e->size;
                    if(howManyTimesIsBiggerThanChainSize%2==0){
                        if(dwellIndex!=0){
                    //    std::cout<<"adding delay"<<dwell_times[dwellIndex]<<" at "<<currentIndex<<"\n";
                    chain->myEvents[currentIndex].data.SendVoidPacketFromLocalNodeData.userIdleTimeAfterRequest=static_cast<uint32_t>(dwell_times[dwellIndex]);
                        }
                    dwellIndex++;
                    }
                }
                ++currentIndex;
            }
        }
        free(e);
        return chain;

    }

   
    
    static EventChain* copyEventChain(EventChain* e) {
        u32 N = e->size;
        EventChain* chain = (EventChain*)malloc(sizeof(EventChain) + (N - 1) * sizeof(Event));
        assertWithMessage(chain != nullptr, "Error, allocation failure");

        chain->size = (u32) N;
        chain->nextEventToExecute = 0;

        for (u32 i = 0; i < e->size; ++ i) {
            chain->myEvents[i] = e->myEvents[i];
            chain->myEvents[i].data.executeNext = chain;
        }
        return chain;
    }
    

};


namespace eventSystem {

    template<usize N>
    void executeEventsInChain(float timeOfExecution, const EventChain::Event(&arr)[N]) {

        if constexpr (N == 1) {
            arr[0].data.executeNext = nullptr;
            addEvent(timeOfExecution, getNextID(), arr[0].data, arr[0].eventFunction );
        }
        else {
            EventChain* chain = EventChain::makeEventChain(arr);
            EventChain::executeNextEventInChain(timeOfExecution, chain);
        }
    }
}

void SendVoidPacketFromISP(f32 timeOfExecution, u32 ID, EventSystemFunctionData data);
void SendVoidPacketFromLocalNode(f32 timeOfExecution, u32 ID, EventSystemFunctionData data);

//Sto event function data, prepei na perno data
void SendVoidPacketFromLocalNode(f32 timeOfExecution, u32 ID, EventSystemFunctionData data) {
    
    //Notify observer about recording
 

    LocalNode* currentNode = data.SendVoidPacketFromLocalNodeData.currentNode;

    if (currentNode->nextAvailableMoment > timeOfExecution) {
        eventSystem::addEvent(currentNode->nextAvailableMoment, ID, data, SendVoidPacketFromLocalNode);
        return;
    }

   if (data.SendVoidPacketFromLocalNodeData.sourceNode == INTERSECTION_ATTACK::attackerNode() &&
        data.SendVoidPacketFromLocalNodeData.isMarked == true) {//Here, when the attacker sends a packet from its Node to the ISP, we enable the flag
            if(INTERSECTION_ATTACK::isRecordingPackets)
            {
                std::cout<<"You have a bug1 \n";
            }
            if(!INTERSECTION_ATTACK::attackerSubmittedAlreadyAPacket)
                std::cout<<"You have a bug3";
            INTERSECTION_ATTACK::attackerSubmittedAlreadyAPacket=false;
        if(PrintAttackerRequestAndResponseTime)
            std::cout<<"starting marking at,"<<timeOfExecution<<"\n";
            lastTimeThatPacketWasSentByAdversary=timeOfExecution;
        INTERSECTION_ATTACK::isRecordingPackets = true;
    }
    LocalNode* destinationServer = data.SendVoidPacketFromLocalNodeData.destinationNode;

    f32 packetSize = data.SendVoidPacketFromLocalNodeData.packetSize;
    f32 timeRequiredToReachISP = packetSize / currentNode->throughput;
    f32 momentWeAreFinishedSending = timeOfExecution + timeRequiredToReachISP;

    currentNode->nextAvailableMoment = momentWeAreFinishedSending;

    //std::cout << " I did my thing";

    //Prepare the next function, which is to send from the ISP to the next target in the path
   // if (INTERSECTION_ATTACK::isRecordingPackets) {
    //    std::cout << "Node with ip" << data.SendVoidPacketFromLocalNodeData.sourceNode->myIP 
     //   << "sent packet to node with ip" << data.SendVoidPacketFromLocalNodeData.destinationNode->myIP << "\n";
  //  }

    EventSystemFunctionData nextData = EventSystemFunctionData {
        .executeNext = data.executeNext,
        .SendVoidPacketFromISPData = {
            .packetSize = packetSize,
            .currentISPIndex = currentNode->myISP,
            .destinationNode = destinationServer,
            .sourceNode = data.SendVoidPacketFromLocalNodeData.sourceNode,
            .userIdleTimeAfterRequest=data.SendVoidPacketFromLocalNodeData.userIdleTimeAfterRequest,
            .isMarked = data.SendVoidPacketFromLocalNodeData.isMarked
        }
    };


    eventSystem::addEvent(momentWeAreFinishedSending, eventSystem::getNextID(), nextData, SendVoidPacketFromISP);
}

void SendVoidPacketFromISP(f32 timeOfExecution, u32 ID, EventSystemFunctionData data) {
    
    

    u32 currentISPIndex = data.SendVoidPacketFromISPData.currentISPIndex;
    LocalNode * srcNode=data.SendVoidPacketFromISPData.sourceNode;
    LocalNode* endReceivingNode = data.SendVoidPacketFromISPData.destinationNode;
    f32 idleTime=data.SendVoidPacketFromISPData.userIdleTimeAfterRequest;
    //Case 1 - We have to send to another ISP
    if (endReceivingNode->myISP != currentISPIndex) {
        EdgeBetweenISPS nextEdge = getNextEdgeInShortestPathBetweenISPS(currentISPIndex, endReceivingNode->myISP);
        f32 momentWeReachTheNextISP = timeOfExecution;
        if(USE_DISTANCE_OVERHEAD_WHEN_SENDING_PACKET_WITH_ISP)
        {
            momentWeReachTheNextISP+= nextEdge.dist;  
        }
            momentWeReachTheNextISP+=CONSTANT_OVERHEAD_WHEN_SENDING_A_PACKET_WITH_ISP; 

        

        //We will pass the same data excluding the currentISP index which is the next one in the path.
        data.SendVoidPacketFromISPData.currentISPIndex = nextEdge.index;

            


        eventSystem::addEvent(momentWeReachTheNextISP, eventSystem::getNextID(), data, SendVoidPacketFromISP);
    }
    else {
        if (endReceivingNode->nextAvailableMoment > timeOfExecution) {
            eventSystem::addEvent(endReceivingNode->nextAvailableMoment, ID, data, SendVoidPacketFromISP);
            return;
        }

        f32 packetSize = data.SendVoidPacketFromISPData.packetSize;
        f32 timeRequiredToReachISP = packetSize / endReceivingNode->throughput;
        f32 momentWeAreFinishedSending = timeOfExecution + timeRequiredToReachISP;

 
        //Now that we are done with this packet sending, we can move on to the next event in the chain
        if (INTERSECTION_ATTACK::isRecordingPackets) {
 
            if(INTERSECTION_ATTACK::attack==AttackType::GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK)
            {
                if(srcNode->type==LocalNodeTag::EntryRelay && allLocalNodes::clients[0]!=endReceivingNode && (endReceivingNode->type!=LocalNodeTag::EntryRelay &&endReceivingNode->type!=LocalNodeTag::MiddleRelay && endReceivingNode->type!=LocalNodeTag::ExitRelay))
                    INTERSECTION_ATTACK::addPacketToGlobalTempSet(srcNode,endReceivingNode);
            }

            if(INTERSECTION_ATTACK::attack==AttackType::PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK)
            {
                if(INTERSECTION_ATTACK::targetNode()==(srcNode))
                {
                    INTERSECTION_ATTACK::addNodeToPartialTempSet(endReceivingNode);
                }
            }
        }
        if (data.SendVoidPacketFromISPData.destinationNode == INTERSECTION_ATTACK::attackerNode() &&
            data.SendVoidPacketFromISPData.isMarked == true ) 
            {
                responseTimeSumOfAdversary+=(momentWeAreFinishedSending-lastTimeThatPacketWasSentByAdversary);
                countOfAdversaryPackets++;

                if(!INTERSECTION_ATTACK::isRecordingPackets)
                    std::cout<<"You have a bug2 \n";
                if(INTERSECTION_ATTACK::attack==AttackType::PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK){
                    INTERSECTION_ATTACK::intersectSetsForRelayLevelIntersectionAttack();
                }
                else if(INTERSECTION_ATTACK::attack==AttackType::GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK){
                    INTERSECTION_ATTACK::intersectPacketsForFullGlobalAdversaryAttack();
                    if(PrintAttackerRequestAndResponseTime)
                         std::cout<<"stoping marking at,"<<timeOfExecution<<"\n";

                }
                INTERSECTION_ATTACK::isRecordingPackets = false;
            
            }

        EventChain::executeNextEventInChain(momentWeAreFinishedSending+idleTime, data.executeNext);
    }


}


EventChain::Event MakeSendVoidPacketFromLocalNodeData(LocalNode* startNode, LocalNode* destinationNode, f32 packetSize, bool isMarked = false,f32 userIdleTimeAfterRequest=0) {
    return EventChain::Event {
        .data = {
            .executeNext = nullptr,
            .SendVoidPacketFromLocalNodeData = {
                .packetSize = packetSize,
                .currentNode = startNode,
                .destinationNode = destinationNode,
                .sourceNode = startNode,
                .userIdleTimeAfterRequest=userIdleTimeAfterRequest,
                .isMarked = isMarked
            }
        },
        .eventFunction = SendVoidPacketFromLocalNode
    };
}


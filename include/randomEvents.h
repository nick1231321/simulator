#pragma once

#include"eventSystem.h"
#include"modernize.h"
#include"mainStructs.h"
#include<random>



inline bool PRINT_SCIENTIFIC_CSV_DATA_ONLY=true;
constexpr f32 TCP_SYN_SIZE=40*8;
constexpr f32 SYN_ACK_SIZE=40*8;
constexpr f32 ACK_SIZE=40*8;
constexpr f32 TLS_CLIENT_HELLO_SIZE=512*8;
constexpr f32 TLS_SERVER_HELLO=512*8;
constexpr f32 TLS_FINISHED=256*8;
constexpr f32 CREATE_TOR_CELL_SIZE=512*8;
constexpr f32 CREATED_TOR_CELL_SIZE=512*8;
constexpr f32 RELAY_EXTEND_SIZE=512*8;
constexpr f32 RELAY_EXTENDED_SIZE=512*8;
constexpr f32 FIN_SIZE=512*8;

constexpr f32 RELAY_INTRODUCE1_SIZE     = 512*8;
constexpr f32 RELAY_RENDEZVOUS1_SIZE    = 512*8;
constexpr f32 RELAY_RENDEZVOUS2_SIZE    = 512*8;
constexpr f32 RELAY_INTRO_ESTABLISHED_SIZE = 512*8;
constexpr f32 ESTABLISH_RENDEVOUZ_SIZE=512*8;
constexpr f32 RENDEVOUZ_ESTABLISHED_SIZE=512*8;
constexpr f32 INTRODUCE1_SIZE=512*8;
constexpr f32 INTRODUCE2_SIZE=512*8;

constexpr f32 INTRODUCE_ACK_SIZE=512*8;
constexpr f32 RENDEVOUZ1_SIZE=512*8;
constexpr f32 RENDEVOUZ2_SIZE=512*8;
constexpr f32 GET_SIZE=512*8;

//Client-Server
//Client-> Entry->Middle->Exit->Server
//CLIENT->ENTRY, ENTRY->MIDDLE, MIDDLE->EXIT, EXIT->SERVER
struct DynamicEventsData{
    int arraySize;
    f32 arr[1];
    LocalNode * src;
    LocalNode * dst;

};
EventChain* SendPacketThroughRelay(LocalNode * sender,LocalNode * receiver,LocalNode * entryRelay,LocalNode *middleRelay,LocalNode * exitRelay,f32 size)
{

    return EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(sender, entryRelay, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelay, middleRelay, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middleRelay, exitRelay, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(exitRelay, receiver, size)  // SYN
    });
}
EventChain* SendPacketThroughRelayAndMarkFirstPacket(LocalNode * sender,LocalNode * receiver,LocalNode * entryRelay,LocalNode *middleRelay,LocalNode * exitRelay,f32 size)
{

    return EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(sender, entryRelay, size,true),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelay, middleRelay, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middleRelay, exitRelay, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(exitRelay, receiver, size)  // SYN
    });
}

EventChain* SendPacketThroughRendevouzCircuit(LocalNode * sender,LocalNode * receiver,LocalNode * entryRelayOfSender,LocalNode *middleRelayOfSender,LocalNode * rendevouzPoint,LocalNode *  middle2RelayOfReceiver,LocalNode *middle1RelayOfReceiver,LocalNode * entryRelayOfReceiver,f32 size,f32 idle_time=0)
{

    return EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(sender, entryRelayOfSender, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelayOfSender, middleRelayOfSender, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middleRelayOfSender, rendevouzPoint, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(rendevouzPoint, middle2RelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middle2RelayOfReceiver, middle1RelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middle1RelayOfReceiver, entryRelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelayOfReceiver, receiver, size,false,idle_time)  // SYN
    });
}
EventChain* SendPacketThroughRendevouzCircuitAndMarkFirstPacket(LocalNode * sender,LocalNode * receiver,LocalNode * entryRelayOfSender,LocalNode *middleRelayOfSender,LocalNode * rendevouzPoint,LocalNode *  middle2RelayOfReceiver,LocalNode *middle1RelayOfReceiver,LocalNode * entryRelayOfReceiver,f32 size )
{

    return EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(sender, entryRelayOfSender, size,true),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelayOfSender, middleRelayOfSender, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middleRelayOfSender, rendevouzPoint, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(rendevouzPoint, middle2RelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middle2RelayOfReceiver, middle1RelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middle1RelayOfReceiver, entryRelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelayOfReceiver, receiver, size)  // SYN
    });
}
EventChain* SendPacketThroughRendevouzCircuitAndMarkLastPacket(LocalNode * sender,LocalNode * receiver,LocalNode * entryRelayOfSender,LocalNode *middleRelayOfSender,LocalNode * rendevouzPoint,LocalNode *  middle2RelayOfReceiver,LocalNode *middle1RelayOfReceiver,LocalNode * entryRelayOfReceiver,f32 size )
{

    return EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(sender, entryRelayOfSender, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelayOfSender, middleRelayOfSender, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middleRelayOfSender, rendevouzPoint, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(rendevouzPoint, middle2RelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middle2RelayOfReceiver, middle1RelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(middle1RelayOfReceiver, entryRelayOfReceiver, size),  // SYN
        MakeSendVoidPacketFromLocalNodeData(entryRelayOfReceiver, receiver, size,true)  // SYN
    });
}
EventChain * TerminateTCPConnection(LocalNode * client, LocalNode * server,LocalNode * entryRelay,LocalNode * middleRelay,LocalNode * exitRelay)
{
    EventChain * FIN1=  SendPacketThroughRelay(client,server,entryRelay,middleRelay,exitRelay,FIN_SIZE);
    EventChain * ACK1=  SendPacketThroughRelay(server,client,exitRelay,middleRelay,entryRelay,FIN_SIZE);
    
    EventChain * FIN2=  SendPacketThroughRelay(server,client,exitRelay,middleRelay,entryRelay,FIN_SIZE);
    EventChain * ACK2=  SendPacketThroughRelay(client,server,entryRelay,middleRelay,exitRelay,ACK_SIZE);

    
    EventChain* ans = EventChain::addEventChains({FIN1, ACK1, FIN2, ACK2});

    //MAYBE I WILL NEED FREE HERE
    return ans;


}
EventChain* EstablishCircuit(LocalNode * currentClient, LocalNode *randomEntryNode,LocalNode * randomMiddleNode,LocalNode * randomExitNode) {
    
    return EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, TCP_SYN_SIZE),  // SYN
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, currentClient, SYN_ACK_SIZE),  // SYN-ACK
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, ACK_SIZE),      // ACK
        // ─── TLS Handshake (Client <-> Entry) ───────── According to https://tpo.pages.torproject.net/core/torspec/tor-spec/channels.html there is a client hello and server hello
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, TLS_CLIENT_HELLO_SIZE),
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, currentClient, TLS_SERVER_HELLO),
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, TLS_FINISHED),
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, currentClient, TLS_FINISHED),
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, CREATE_TOR_CELL_SIZE),   // CREATE1
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, currentClient, CREATED_TOR_CELL_SIZE),  // CREATED1
        // ─── Extend to Middle (OR2) ───────────────────
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, RELAY_EXTEND_SIZE),       // RELAY_EXTEND (→ OR2)
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, randomMiddleNode, CREATE_TOR_CELL_SIZE),     // RELAY_EXTENDED
        MakeSendVoidPacketFromLocalNodeData(randomMiddleNode, randomEntryNode, CREATED_TOR_CELL_SIZE),     // RELAY_EXTENDED
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, currentClient, CREATED_TOR_CELL_SIZE),     // RELAY_EXTENDED
        // ─── Extend to Exit (OR3) ───────────────────
        MakeSendVoidPacketFromLocalNodeData(currentClient, randomEntryNode, RELAY_EXTEND_SIZE),       // RELAY_EXTEND (→ OR2)
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, randomMiddleNode, RELAY_EXTEND_SIZE),     // RELAY_EXTENDED
        MakeSendVoidPacketFromLocalNodeData(randomMiddleNode, randomExitNode, CREATE_TOR_CELL_SIZE),     // RELAY_EXTENDED
        MakeSendVoidPacketFromLocalNodeData(randomExitNode, randomMiddleNode, CREATED_TOR_CELL_SIZE),     // RELAY_EXTENDED
        MakeSendVoidPacketFromLocalNodeData(randomMiddleNode, randomEntryNode, CREATED_TOR_CELL_SIZE),     // RELAY_EXTENDED
        MakeSendVoidPacketFromLocalNodeData(randomEntryNode, currentClient, CREATED_TOR_CELL_SIZE)     // RELAY_EXTENDED
    });

}
// Picks unique indices from [0, n) with Bernoulli(p) distribution for i>0
// Always includes index 0.
std::vector<uint32_t> pickIndicesBinomial(uint32_t n, double p) {
    std::vector<uint32_t> out;
    if (n == 0) return out;

    // Always include index 0
    out.push_back(0);

    if (n == 1 || p <= 0.0) {
        return out; // nothing else to add
    }

    uint32_t start = 1u; // start sampling from index 1
    auto& rng = RNG::getEngine();

    // Step 1: Draw K ~ Binomial(n-1, p) for the remaining indices
    std::binomial_distribution<uint32_t> binom(n - start, p);
    uint32_t K = binom(rng);
    if (K == 0) {
        return out;
    }

    // Step 2: Floyd’s algorithm to pick K unique numbers in [start, n-1]
    std::unordered_set<uint32_t> S;
    S.reserve(K * 2);
    for (uint32_t t = n - 1; S.size() < K; --t) {
        std::uniform_int_distribution<uint32_t> U(start, t);
        uint32_t r = U(rng);
        if (!S.insert(r).second) {
            // collision: take t instead
            S.insert(t);
        }
    }

    // Step 3: Combine 0 with chosen indices, sorted
    out.insert(out.end(), S.begin(), S.end());
    std::sort(out.begin(), out.end());
    return out;
}

inline void randomEventsFunction(f32 timeOfExecution, u32 ID, EventSystemFunctionData data) {
//auto selected = pickIndicesBinomial(allLocalNodes::allNodePops.clientsPop,
 //                                   userRequestProbability);

    for (u32 i = 0; i < allLocalNodes::allNodePops.clientsPop; ++i) { // 0th node is the node that performs the attack so we don't send random events

        //an ginete na vazume ton kathe server i anonymous service na exei mia pithanotita na epilexthei.
        if(i!=0 && !RNG::coinFlip(userRequestProbability)){
            if(PRINT_DEBUG_MESSAGES)
                std::cout<<"Skipping making a user request\n";
            continue;
        }
        if(i==0 && (INTERSECTION_ATTACK::isRecordingPackets || INTERSECTION_ATTACK::attackerSubmittedAlreadyAPacket)){
          //  std::cout<<"Skipping Making a request for anonymous service.Flag is up\n";
            continue;
        }
        bool makeRequestToRegularServer=RNG::coinFlip(probabilityToMakeRequestToAnAnonymousService);
        if(i==0){
            makeRequestToRegularServer=false;
        }
        if (makeRequestToRegularServer) { 
            if(PRINT_DEBUG_MESSAGES)
                {
                    std::cout<<"Making a request to a random server\n";
                }
                
            LocalNode* currentClient    = allLocalNodes::clients[i];
            LocalNode* randomServer     = allLocalNodes::servers[RNG::getRandomIndexForTag(LocalNodeTag::Server)];
            LocalNode* entryGuardRelay  = entryGuardsOfClients[i];
            LocalNode* randomMiddleNode = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];
            LocalNode* randomExitNode   = allLocalNodes::exitRelays[RNG::getRandomIndexForTag(LocalNodeTag::ExitRelay)];
            
            EventChain * establishingCircuitsEvents = EstablishCircuit(currentClient,entryGuardRelay,randomMiddleNode,randomExitNode);
            f32 noise=1000;
            
            EventChain * sendingPacketEvents= SendPacketThroughRelay(currentClient,randomServer,entryGuardRelay,randomMiddleNode,randomExitNode,GET_SIZE);

            EventChain * sendingPacketEvents2= SendPacketThroughRelay(randomServer,currentClient,randomExitNode,randomMiddleNode,entryGuardRelay,noise);
            //TerminateTCPConnection(LocalNode * client, LocalNode * server,LocalNode * entryRelay,LocalNode * middleRelay,LocalNode * exitRelay)
            EventChain * TerminateTCPConnection1= SendPacketThroughRelay(currentClient,randomServer,entryGuardRelay,randomMiddleNode,randomExitNode,FIN_SIZE);
            EventChain * TerminateTCPConnection2= SendPacketThroughRelay(randomServer,currentClient,randomExitNode,randomMiddleNode,entryGuardRelay,ACK_SIZE);
            EventChain * TerminateTCPConnection3= SendPacketThroughRelay(randomServer,currentClient,randomExitNode,randomMiddleNode,entryGuardRelay,FIN_SIZE);
            EventChain * TerminateTCPConnection4= SendPacketThroughRelay(currentClient,randomServer,entryGuardRelay,randomMiddleNode,randomExitNode,ACK_SIZE);

            EventChain* ans = EventChain::addEventChains({establishingCircuitsEvents, sendingPacketEvents, sendingPacketEvents2, TerminateTCPConnection1,TerminateTCPConnection2,TerminateTCPConnection3,TerminateTCPConnection4});
            establishingCircuitsEvents=nullptr;
            sendingPacketEvents=nullptr;
            sendingPacketEvents2=nullptr;
            TerminateTCPConnection1=nullptr;
                        TerminateTCPConnection2=nullptr;
            TerminateTCPConnection3=nullptr;
            TerminateTCPConnection4=nullptr;

            EventChain::executeNextEventInChain(timeOfExecution, ans);
        }
        else {

                if(PRINT_DEBUG_MESSAGES)
                {
                    std::cout<<"Making a request to a random  anonymous server\n";
                }
            

            u32 onionServiceIndex;
            if(i!=0)
                onionServiceIndex= RNG::getRandomIndexForTag(LocalNodeTag::AnonymousService);//rand() % allLocalNodes::allNodePops.anonymousServicesPop;
            else{
                INTERSECTION_ATTACK::attackerSubmittedAlreadyAPacket=true;
                onionServiceIndex=INTERSECTION_ATTACK::targetAnonymousService;
            }

            LocalNode* currentClient    = allLocalNodes::clients[i];
            LocalNode* onionService     = allLocalNodes::anonymousServices[onionServiceIndex];
            
            int index;
            if(i!=0)
                index=rand() % INTRODUCTION_POINTS_PER_ANONYMOUS_SERVICE;
            else
                index=0;
            // Introduction Point Circuit Relays

            //This is the nodes of the circuit from the introduction point to the sanonymous service.
            LocalNode * hiddenEntryRelayOfIntroductionPoint=anonymousServicesIntroductionPoints[onionServiceIndex][index].EntryRelay;
            LocalNode * hiddenMiddleRelayOfIntroductionPoint=anonymousServicesIntroductionPoints[onionServiceIndex][index].MiddleRelay1;
            LocalNode * introductionPoint = anonymousServicesIntroductionPoints[onionServiceIndex][index].MiddleRelay2;//NICOLAS: TA introduction points prepei na einai middle relays

            //Circuit from client to Rendevouz Point
            LocalNode* firstMiddleRelayOfRendevouzPointOfClient    = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];
            LocalNode* RendevouzPoint  = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];

            // These relays will be used by client for connecting with IP
            LocalNode* entryGuardRelay    = entryGuardsOfClients[i];
            LocalNode* firstMiddleRelayOfClientToIP    = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];
            LocalNode* secondMiddleRelayOfClientToIP   = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];
            
           //These will be used by anonymous service for establishing a connection with AS 
            LocalNode* RelayOfRendevouzCircuitofOnionServer1     = hiddenEntryRelayOfIntroductionPoint;
            LocalNode* RelayOfRendevouzCircuitofOnionServer2    = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];
            LocalNode* RelayOfRendevouzCircuitofOnionServer3    = allLocalNodes::middleRelays[RNG::getRandomIndexForTag(LocalNodeTag::MiddleRelay)];

            EventChain * establishingRendevouzPointClientEvents=EstablishCircuit(currentClient,entryGuardRelay,firstMiddleRelayOfRendevouzPointOfClient,RendevouzPoint);
            EventChain * establishRendevouzEvents=EventChain::makeEventChain({
        MakeSendVoidPacketFromLocalNodeData(currentClient, entryGuardRelay, ESTABLISH_RENDEVOUZ_SIZE),  
        MakeSendVoidPacketFromLocalNodeData(entryGuardRelay, firstMiddleRelayOfRendevouzPointOfClient, ESTABLISH_RENDEVOUZ_SIZE),  
        MakeSendVoidPacketFromLocalNodeData(firstMiddleRelayOfRendevouzPointOfClient, RendevouzPoint, ESTABLISH_RENDEVOUZ_SIZE),  
    });
    
        EventChain * rendevouzEstablishedEvents=EventChain::makeEventChain({
                MakeSendVoidPacketFromLocalNodeData(RendevouzPoint, firstMiddleRelayOfRendevouzPointOfClient, RENDEVOUZ_ESTABLISHED_SIZE),  
                MakeSendVoidPacketFromLocalNodeData(firstMiddleRelayOfRendevouzPointOfClient, entryGuardRelay, RENDEVOUZ_ESTABLISHED_SIZE),  
                MakeSendVoidPacketFromLocalNodeData(entryGuardRelay, currentClient, RENDEVOUZ_ESTABLISHED_SIZE),  
        });

        
        EventChain * establishCircuitForCommunicatingWithIntroductionPoint=EstablishCircuit(currentClient,entryGuardRelay,firstMiddleRelayOfClientToIP,secondMiddleRelayOfClientToIP);
       
       EventChain * introduce1Events=nullptr;
       if(i==0 && INTERSECTION_ATTACK::attack==AttackType::PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK)
            introduce1Events=SendPacketThroughRelayAndMarkFirstPacket(currentClient,introductionPoint,entryGuardRelay,firstMiddleRelayOfClientToIP,secondMiddleRelayOfClientToIP,INTRODUCE1_SIZE);
        else
            introduce1Events=SendPacketThroughRelay(currentClient,introductionPoint,entryGuardRelay,firstMiddleRelayOfClientToIP,secondMiddleRelayOfClientToIP,INTRODUCE1_SIZE);

        EventChain * introduce2Events=EventChain::makeEventChain({
                            MakeSendVoidPacketFromLocalNodeData(introductionPoint, hiddenMiddleRelayOfIntroductionPoint, INTRODUCE2_SIZE),//todo:na to valo alou, sto proigumeno  
                            MakeSendVoidPacketFromLocalNodeData(hiddenMiddleRelayOfIntroductionPoint, hiddenEntryRelayOfIntroductionPoint, INTRODUCE2_SIZE),  
                            MakeSendVoidPacketFromLocalNodeData(hiddenEntryRelayOfIntroductionPoint, onionService, INTRODUCE2_SIZE), 
        });
        EventChain * introduceACKEvents;
        if(i==0 && INTERSECTION_ATTACK::attack==AttackType::PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK){
            introduceACKEvents=EventChain::makeEventChain({
            MakeSendVoidPacketFromLocalNodeData(onionService, hiddenEntryRelayOfIntroductionPoint, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(hiddenEntryRelayOfIntroductionPoint, hiddenMiddleRelayOfIntroductionPoint, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(hiddenMiddleRelayOfIntroductionPoint, introductionPoint, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(introductionPoint, secondMiddleRelayOfClientToIP, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(secondMiddleRelayOfClientToIP, firstMiddleRelayOfClientToIP, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(firstMiddleRelayOfClientToIP, entryGuardRelay, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(entryGuardRelay, currentClient, INTRODUCE_ACK_SIZE,true), 
        });
    }
    else
    {
            introduceACKEvents=EventChain::makeEventChain({
            MakeSendVoidPacketFromLocalNodeData(onionService, hiddenEntryRelayOfIntroductionPoint, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(hiddenEntryRelayOfIntroductionPoint, hiddenMiddleRelayOfIntroductionPoint, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(hiddenMiddleRelayOfIntroductionPoint, introductionPoint, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(introductionPoint, secondMiddleRelayOfClientToIP, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(secondMiddleRelayOfClientToIP, firstMiddleRelayOfClientToIP, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(firstMiddleRelayOfClientToIP, entryGuardRelay, INTRODUCE_ACK_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(entryGuardRelay, currentClient, INTRODUCE_ACK_SIZE), 
        });   
    }

        EventChain * establishCircuitFromOnionServiceToRendevouzPoint=EstablishCircuit(onionService,RelayOfRendevouzCircuitofOnionServer1,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer3);
        EventChain * sendRendevouz1ToRendevouzPointFromOnionServer=EventChain::makeEventChain({
            MakeSendVoidPacketFromLocalNodeData(onionService, RelayOfRendevouzCircuitofOnionServer1, RENDEVOUZ1_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(RelayOfRendevouzCircuitofOnionServer1, RelayOfRendevouzCircuitofOnionServer2, RENDEVOUZ1_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(RelayOfRendevouzCircuitofOnionServer2, RelayOfRendevouzCircuitofOnionServer3, RENDEVOUZ1_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(RelayOfRendevouzCircuitofOnionServer3, RendevouzPoint, RENDEVOUZ1_SIZE), 

        });
        EventChain * sendRendevouz2FromRendevouzPointToClient=EventChain::makeEventChain({
            MakeSendVoidPacketFromLocalNodeData(RendevouzPoint, firstMiddleRelayOfRendevouzPointOfClient, RENDEVOUZ2_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(firstMiddleRelayOfRendevouzPointOfClient, entryGuardRelay, RENDEVOUZ2_SIZE), 
            MakeSendVoidPacketFromLocalNodeData(entryGuardRelay, currentClient, RENDEVOUZ2_SIZE), 

        });
        f32 noise=1024;
      //  EventChain * SendGetByClientToOnionServiceByUsingRendevouzCircuit;
      //  EventChain * receiveNoiseFromOnionServiceToClient;
        EventChain * sendAndReceivePacketsFromRendevouzCircuit=NULL;
        if(i==0 && INTERSECTION_ATTACK::attack==AttackType::GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK)
        {

                EventChain * SendGetByClientToOnionServiceByUsingRendevouzCircuit=SendPacketThroughRendevouzCircuitAndMarkFirstPacket(currentClient,onionService,entryGuardRelay,firstMiddleRelayOfRendevouzPointOfClient,RendevouzPoint,RelayOfRendevouzCircuitofOnionServer3,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer1,GET_SIZE);
                EventChain * receiveNoiseFromOnionServiceToClient=SendPacketThroughRendevouzCircuitAndMarkLastPacket(onionService,currentClient,RelayOfRendevouzCircuitofOnionServer1,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer3,RendevouzPoint,firstMiddleRelayOfRendevouzPointOfClient,entryGuardRelay,noise);
                sendAndReceivePacketsFromRendevouzCircuit=EventChain::addEventChains({SendGetByClientToOnionServiceByUsingRendevouzCircuit,receiveNoiseFromOnionServiceToClient});
        }
        else
        {

            EventChain * SendGetByClientToOnionServiceByUsingRendevouzCircuit=SendPacketThroughRendevouzCircuit(currentClient,onionService,entryGuardRelay,firstMiddleRelayOfRendevouzPointOfClient,RendevouzPoint,RelayOfRendevouzCircuitofOnionServer3,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer1,GET_SIZE);
            EventChain *receiveNoiseFromOnionServiceToClient=SendPacketThroughRendevouzCircuit(onionService,currentClient,RelayOfRendevouzCircuitofOnionServer1,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer3,RendevouzPoint,firstMiddleRelayOfRendevouzPointOfClient,entryGuardRelay,noise);
            EventChain * tmp=EventChain::addEventChains({SendGetByClientToOnionServiceByUsingRendevouzCircuit,receiveNoiseFromOnionServiceToClient});
            if(makeASingleRequest || i==0){
                sendAndReceivePacketsFromRendevouzCircuit=tmp;
            }
            else
            {//We simulate user behavior
            auto s = UserSessionSimulator::simulateSession();
                sendAndReceivePacketsFromRendevouzCircuit= EventChain::multiplyBy(tmp,static_cast<u32>(s.pages),s.dwell_times);
            }
        }
        //FIN
        EventChain * FIN1=SendPacketThroughRendevouzCircuit(currentClient,onionService,entryGuardRelay,firstMiddleRelayOfRendevouzPointOfClient,RendevouzPoint,RelayOfRendevouzCircuitofOnionServer3,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer1,FIN_SIZE);
        EventChain * ACK1=SendPacketThroughRendevouzCircuit(onionService,currentClient,RelayOfRendevouzCircuitofOnionServer1,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer3,RendevouzPoint,firstMiddleRelayOfRendevouzPointOfClient,entryGuardRelay,ACK_SIZE);
        EventChain * FIN2=SendPacketThroughRendevouzCircuit(onionService,currentClient,RelayOfRendevouzCircuitofOnionServer1,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer3,RendevouzPoint,firstMiddleRelayOfRendevouzPointOfClient,entryGuardRelay,FIN_SIZE);
        EventChain * ACK2=SendPacketThroughRendevouzCircuit(currentClient,onionService,entryGuardRelay,firstMiddleRelayOfRendevouzPointOfClient,RendevouzPoint,RelayOfRendevouzCircuitofOnionServer3,RelayOfRendevouzCircuitofOnionServer2,RelayOfRendevouzCircuitofOnionServer1,ACK_SIZE);
        EventChain* ans = EventChain::addEventChains({establishingRendevouzPointClientEvents, establishRendevouzEvents, rendevouzEstablishedEvents, establishCircuitForCommunicatingWithIntroductionPoint,introduce1Events,introduce2Events,introduceACKEvents,establishCircuitFromOnionServiceToRendevouzPoint,sendRendevouz1ToRendevouzPointFromOnionServer,sendRendevouz2FromRendevouzPointToClient,sendAndReceivePacketsFromRendevouzCircuit,FIN1,ACK1,FIN2,ACK2});

        EventChain::executeNextEventInChain(timeOfExecution, ans);
        }
    }
}

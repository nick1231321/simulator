#pragma once
#include"modernize.h"
#include<vector>
#include<array>
#include <unordered_set>
#include <fstream>
#include <string>
#include <iostream>


/*
Assumptions:
1) ISPs have unlimited throughput
2) The bandwith each currentClient uses to send and receive are separate
3) A currentClient can be reiceving or send one package at a time
4) When you select a randomServer/ or a relay there is no overhead
Node 

*/
/*
enum Flag {
   //to implement after 
    test    = 1 << 0, // 0b0001
};*/
#include <unordered_map>
inline bool makeASingleRequest=false;

union IpAdress;
struct ISP;
struct EdgeBetweenISPS {
    u32 index;
    u32 dist;
};
struct LocalNode;
struct Packet;

union IpAdress {
    
    u8 nums[4]; //0 - 255 
    u32 rawNumber;

    friend std::ostream& operator<<(std::ostream& os, const IpAdress& i) {
        os << (int)i.nums[0] << "." << (int)i.nums[1] << "." << (int)i.nums[2] << "." << (int)i.nums[3];
        return os;
    }
};

struct ISP {
    u32 clientPop;
    LocalNode* myClients;
    IpAdress baseIp;
    u8 numberOfFixedBitsInIP;  
};

constexpr u32 INF = 0x3f3f3f3f; //00111111 00111111 00111111 00111111
constexpr u32 MAX_ISP_NUM = 1005;
constexpr u8 MAX_DEGREE_OF_FREEDOM = 16;
constexpr u8 MIN_DEGREE_OF_FREEDOM = 8;
constexpr int EXIT_AFTER_STACKING_AT_N_ITERATIONS=100;
inline bool USE_DISTANCE_OVERHEAD_WHEN_SENDING_PACKET_WITH_ISP=false;
inline f32 CONSTANT_OVERHEAD_WHEN_SENDING_A_PACKET_WITH_ISP=1;

inline ISP arrayISP[MAX_ISP_NUM]; 
inline u32 popISP;
inline std::vector<EdgeBetweenISPS> edgesBetweenISPS[MAX_ISP_NUM];
inline u32 neighboringDistanceBetweenISPS[MAX_ISP_NUM][MAX_ISP_NUM];
inline u32 shortestPathBetweenISPS[MAX_ISP_NUM][MAX_ISP_NUM];
inline u32 nextInShortestPathBetweenISPS[MAX_ISP_NUM][MAX_ISP_NUM];
//These variables 
inline f32 lastTimeThatPacketWasSentByAdversary=0;
inline f32 responseTimeSumOfAdversary=0;
inline u32 countOfAdversaryPackets=0;

enum class LocalNodeTag : u32
{
    EntryRelay, //0 
    MiddleRelay,
    ExitRelay,
    Client,
    Server,
    AnonymousService, //5
    _COUNT 
}; // N nodes  - int[6] 0 2 3 1 3 5 

constexpr std::string_view toString(LocalNodeTag tag) {
    switch (tag) {
        case LocalNodeTag::EntryRelay:       return "EntryRelay";
        case LocalNodeTag::MiddleRelay:      return "MiddleRelay";
        case LocalNodeTag::ExitRelay:        return "ExitRelay";
        case LocalNodeTag::Client:           return "Client";
        case LocalNodeTag::Server:           return "Server";
        case LocalNodeTag::AnonymousService: return "AnonymousService";
        default:                             return "Unknown";
    }
}
// Optional: operator<< overload for direct streaming
std::ostream& operator<<(std::ostream& os, LocalNodeTag tag) {
    return os << toString(tag);
}

constexpr u32 LocalNodeTagPop = (u32)LocalNodeTag::_COUNT;

union localNodeTagDistribution {
    struct {
        u32 entryRelaysPop;
        u32 middleRelaysPop;
        u32 exitRelaysPop;
        u32 clientsPop;
        u32 serversPop;
        u32 anonymousServicesPop;
    };
    u32 arr[LocalNodeTagPop];
};
double userRequestProbability;//This is the probability that user makes a request to either an anonymous service, or a server.
double probabilityToMakeRequestToAnAnonymousService;
constexpr u32 MAX_ENTRY_RELAYS_POP = 6000;
constexpr u32 MAX_MIDDLE_RELAYS_POP = 6000;    
constexpr u32 MAX_EXIT_RELAYS_POP = 6000;
constexpr u32 MAX_CLIENT_POP = 2000000;
constexpr u32 MAX_SERVER_POP = 5000000;
constexpr u32 MAX_ANONYMOUS_SERVICES_POP = 200000; 
constexpr bool PRINT_DEBUG_MESSAGES = false; 

constexpr u8 INTRODUCTION_POINTS_PER_ANONYMOUS_SERVICE = 3;
constexpr u8 CIRCUIT_LENGTH_TO_DEANONYMIZE = 4;
LocalNode * entryGuardsOfClients[MAX_CLIENT_POP];
bool doNotUseEntryGuard=true;
namespace ATTACK_LOGGER {

    inline std::ofstream csvFile;
    inline std::string currentCsvName; // Store the name for creating extra files

    inline void open(const std::string& filename) {
        currentCsvName = filename; // Save for later

        csvFile.open(filename, std::ios::out);
        if (!csvFile.is_open()) {
            std::cerr << "Error: Could not open CSV log file: " << filename << "\n";
            std::exit(1);
        }
    }

    inline void logLine(const std::string& line) {
        if (csvFile.is_open()) {
            csvFile << line << "\n";
        }
    }

    inline void close() {
        if (csvFile.is_open()) {
            csvFile.close();
        }
    }

}

inline struct {
    LocalNode* EntryRelay;
    LocalNode* MiddleRelay1;
    LocalNode* MiddleRelay2;
}
anonymousServicesIntroductionPoints[MAX_ANONYMOUS_SERVICES_POP][INTRODUCTION_POINTS_PER_ANONYMOUS_SERVICE];



namespace allLocalNodes {
    inline u32 totalLocalNodesPop;

    
 
    inline localNodeTagDistribution allNodePops;

    inline LocalNode* entryRelays[MAX_ENTRY_RELAYS_POP];
    inline LocalNode* middleRelays[MAX_MIDDLE_RELAYS_POP];
    inline LocalNode* exitRelays[MAX_EXIT_RELAYS_POP];
    inline LocalNode* clients[MAX_CLIENT_POP]; 
    inline LocalNode* servers[MAX_SERVER_POP];
    inline LocalNode* anonymousServices[MAX_ANONYMOUS_SERVICES_POP];
    // anonymousServices[0] = target
    // client[0] = attacker




    inline LocalNode** as2DArray[] = {
        entryRelays,
        middleRelays,
        exitRelays,
        clients,
        servers,
        anonymousServices
    };
}




struct LocalNode {
    u32 myISP; //index in global array
    f32 throughput;
    IpAdress myIP; //My IP
    LocalNodeTag type;
    f32 nextAvailableMoment;
    //Flag myFlag;
    //when am i done receiving ready or time
};
inline std::string localNodeTagToString(LocalNodeTag tag) {
    switch (tag) {
        case LocalNodeTag::Client: return "Client";
        case LocalNodeTag::AnonymousService: return "AnonymousService";
        case LocalNodeTag::EntryRelay: return "EntryRelay";
        case LocalNodeTag::MiddleRelay: return "MiddleRelay";
        case LocalNodeTag::ExitRelay: return "ExitRelay";
        case LocalNodeTag::Server: return "Server";
        default: return "Unknown";
    }
}


enum AttackType {
    GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK,
    PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK
};
inline bool PrintAttackerRequestAndResponseTime=false;
inline const char* attackTypeToString(AttackType type) {
    switch (type) {
        case GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK:
            return "global";
        case PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK:
            return "relay";
        default:
            return "unknown";
    }
}

namespace INTERSECTION_ATTACK {
    AttackType attack = GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK;
    inline bool attackerSubmittedAlreadyAPacket=false;
    inline bool isRecordingPackets = false;
    inline std::unordered_map<LocalNode*, size_t> entryRelayMap;
    inline int epoch=0;
    inline int sumOfStackedEpochs=0;
    inline int previousIntersectionLength=0;
    inline u32 targetAnonymousService=0;
    inline LocalNode * victimNode=NULL;
    // Relay Level attack Data structures
    inline std::unordered_set<LocalNode*> observed_set;
    inline std::unordered_set<LocalNode*> temp_set;
    inline std::vector<LocalNode*> identified_nodes;//
inline void writeStackedNodesFile(const std::unordered_set<LocalNode*>& nodes) {
    // Build filename
    std::string stackedFile = ATTACK_LOGGER::currentCsvName;
    size_t pos = stackedFile.find_last_of('.');
    if (pos != std::string::npos) {
        stackedFile.insert(pos, "_stacked_nodes");
    } else {
        stackedFile += "_stacked_nodes.csv";
    }

    std::ofstream out(stackedFile, std::ios::out);
    if (!out.is_open()) {
        std::cerr << "Error: Could not open stacked nodes file: " << stackedFile << "\n";
        return;
    }

    // Count occurrences of each node type
    std::unordered_map<LocalNodeTag, size_t> typeCounts;
    for (auto* node : nodes) {
        if (node) {
            typeCounts[node->type]++;
        }
    }

    // Write header
    out << "node_type,count\n";

    // Write each type and its count
    for (const auto& [type, count] : typeCounts) {
        out << localNodeTagToString(type) << "," << count << "\n";
    }

    out.close();
}


inline void logRemainingNodeTypesAndExit(const std::unordered_set<LocalNode*>& nodes, int exitCode) {
    ATTACK_LOGGER::logLine("remaining_node_types");
    for (auto* node : nodes) {
        if (node) {
            ATTACK_LOGGER::logLine(localNodeTagToString(node->type));
        }
    }
    ATTACK_LOGGER::close();
    std::exit(exitCode);
}

    // Global adversary attack data
    using PacketKey = std::pair<LocalNode*, LocalNode*>;
    struct PacketKeyHash {
        std::size_t operator()(const PacketKey& p) const noexcept {
            return std::hash<LocalNode*>()(p.first) ^ (std::hash<LocalNode*>()(p.second) << 1);
        }
    };
    inline std::unordered_set<PacketKey, PacketKeyHash> observed_packets;
    inline std::unordered_set<PacketKey, PacketKeyHash> temp_packets;


    inline void printSet(const std::unordered_set<LocalNode*>& s, const std::string& name) {
        std::cout << name << ": ";
        for (LocalNode * node : s) {
            std::cout << node->myIP << " ";
        }
        std::cout << "\n";
    }

    // Partial adversary attack: same behavior as before
    inline void addNodeToPartialTempSet(LocalNode* node) {
        if (node == NULL) {
            std::cout << "Node is null\n";
            return;
        }
        if (attack == PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK) {
            if (std::find(identified_nodes.begin(), identified_nodes.end(), node) != identified_nodes.end()) {
                return;
            }
        }
        temp_set.insert(node);
    }

    // Global adversary attack: store src/dst pair
    inline void addPacketToGlobalTempSet(LocalNode* src, LocalNode* dst) {
        if (src == NULL || dst == NULL) {
            std::cout << "Src or Dst is null\n";
            return;
        }
        temp_packets.insert({src, dst});
    }

    void initializeExitRelayMap() {
        for (size_t i = 0; i < MAX_EXIT_RELAYS_POP; ++i) {
            if (allLocalNodes::exitRelays[i] != nullptr) {
                entryRelayMap[allLocalNodes::entryRelays[i]] = i; // store index
            }
        }
    }
// Returns true if node exists in the map (i.e., is an exit relay)
inline bool isEntryRelay(LocalNode* node) {
    return entryRelayMap.find(node) != entryRelayMap.end();
}


    // Global adversary intersection
    inline void intersectPacketsForFullGlobalAdversaryAttack() {
        if (observed_packets.empty()) {
            observed_packets = std::move(temp_packets);
            previousIntersectionLength=observed_packets.size();
        } else {
            std::unordered_set<PacketKey, PacketKeyHash> new_packets;
            for (auto& pkt : observed_packets) {
                if (temp_packets.find(pkt) != temp_packets.end()) {
                    new_packets.insert(pkt);
                }
            }
           
            observed_packets.swap(new_packets);
            temp_packets.clear();
        if(previousIntersectionLength==observed_packets.size())
            {
                sumOfStackedEpochs++;
                if(sumOfStackedEpochs==EXIT_AFTER_STACKING_AT_N_ITERATIONS)
                {
                float avgRT = responseTimeSumOfAdversary / countOfAdversaryPackets;
                    ATTACK_LOGGER::logLine(
                        "avg_response_time," + std::to_string(avgRT));

                    ATTACK_LOGGER::logLine(
                        "successness," + std::to_string(false)
                    );
                    ATTACK_LOGGER::close();
                std::unordered_set<LocalNode*> remainingNodes;

                    for (auto& pkt : observed_packets) {
                        remainingNodes.insert(pkt.second); // or pkt.first depending on interest
                    }
                    writeStackedNodesFile(remainingNodes);


                    exit(-1);
                }
            }
            else
            {
                sumOfStackedEpochs=0;
                previousIntersectionLength=observed_packets.size();
            }
        }

ATTACK_LOGGER::logLine(
    std::to_string(epoch) + "," + std::to_string(observed_packets.size())
);
if((epoch%5==0) && epoch>5 ){
        std::cout<<std::to_string(epoch) + "," + std::to_string(observed_packets.size())<<std::endl;
}

        if (observed_packets.size() == 1) {
            auto found = *observed_packets.begin();
    float avgRT = responseTimeSumOfAdversary / countOfAdversaryPackets;
    bool success = (found.second == allLocalNodes::anonymousServices[targetAnonymousService]);

    ATTACK_LOGGER::logLine(
        "successness," + std::to_string(success)
    );

    ATTACK_LOGGER::logLine(
        "avg_response_time," + std::to_string(avgRT));
            std::exit(1);
        }
        if (observed_packets.empty()) {
            std::cout << "error: observed packets size is 0";
            exit(-1);
        }
        epoch++;
    }

    // Intersect observed_set with temp_set and handle logic   // Intersect observed_set with temp_set and handle logic
    inline void intersectSetsForRelayLevelIntersectionAttack() {
        
        if (observed_set.empty()) {
            // First time: observed_set takes temp_set entirely
            observed_set = std::move(temp_set);
                previousIntersectionLength=observed_set.size();

        } else {
            // Intersect observed_set with temp_set
            std::unordered_set<LocalNode*> new_set;

            for (auto* node : observed_set) {
                if (temp_set.find(node) != temp_set.end()) {
                    new_set.insert(node);
                }
            }

            observed_set.swap(new_set);
            temp_set.clear();
        }
        epoch++;
        if((epoch%5==0 || observed_set.size()==1)){
        std::cout<<std::to_string(epoch) + "," +
            std::to_string(identified_nodes.size()-1) + "," +
            std::to_string(observed_set.size())<<std::endl;
        }

        ATTACK_LOGGER::logLine(
            std::to_string(epoch) + "," +
            std::to_string(identified_nodes.size()-1) + "," +
            std::to_string(observed_set.size())
        );
        // Check if observed_set size is 1
        if(observed_set.size()==0)
        {
            std::cout<<"error:observed size is 0";
            exit(-1);
        }
        else if (observed_set.size() == 1) {
            // Save the node
            epoch=0;
            LocalNode* found = *observed_set.begin();
      
            identified_nodes.push_back(found);
             if((identified_nodes.size()-1)==1)
            {

                ATTACK_LOGGER::logLine(
                    "successness," + std::to_string(found==anonymousServicesIntroductionPoints[0][0].MiddleRelay1)
                );

            }
            else if((identified_nodes.size()-1)==2)
            {
                                ATTACK_LOGGER::logLine(
                    "successness," + std::to_string(found==anonymousServicesIntroductionPoints[0][0].EntryRelay)
                );
            }
            else if((identified_nodes.size()-1)==3)
            {
                                ATTACK_LOGGER::logLine(
                    "successness," + std::to_string(found==allLocalNodes::anonymousServices[targetAnonymousService])
                );
            float avgRT = responseTimeSumOfAdversary / countOfAdversaryPackets;
                          ATTACK_LOGGER::logLine(
                    "avg_response_time," + std::to_string(avgRT)
                );
                ATTACK_LOGGER::close();
                exit(1);
            }

            // Reset observed_set and temp_set
            observed_set.clear();
            temp_set.clear();
        }
        if(observed_set.size()==previousIntersectionLength)
        {
            sumOfStackedEpochs++;
        if(sumOfStackedEpochs==EXIT_AFTER_STACKING_AT_N_ITERATIONS)
                {
                float avgRT = responseTimeSumOfAdversary / countOfAdversaryPackets;
                    ATTACK_LOGGER::logLine(
                        "avg_response_time," + std::to_string(avgRT));

                    ATTACK_LOGGER::logLine(
                        "successness," + std::to_string(false)
                    );
                    ATTACK_LOGGER::close();
        writeStackedNodesFile(observed_set); // New file with stacked node types

                    //logRemainingNodeTypesAndExit(observed_set, -1);

                    exit(-1);
                }
            }
            else
            {
                sumOfStackedEpochs=0;
                previousIntersectionLength=observed_set.size();
            }
        }

    inline LocalNode* attackerNode() {
        return allLocalNodes::clients[0];
    }
inline LocalNode* targetNode() {
    // If the list is empty, push the fallback node first
    if (identified_nodes.empty() && AttackType::PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK==attack) {
        LocalNode* fallback = anonymousServicesIntroductionPoints[0][0].MiddleRelay2;
        identified_nodes.push_back(fallback);
    }

    // Return the last (most recently identified) node
    return identified_nodes.back();
}

}
struct Packet
{
    const LocalNode& src;
    const LocalNode& dst;
    f32 size;
};

//Returns only the value of the shortest path
inline u32 getShortestDistanceBetweenISPS(u32 isp1, u32 isp2) {
    return shortestPathBetweenISPS[isp1][isp2];
}

inline EdgeBetweenISPS getNextEdgeInShortestPathBetweenISPS(u32 isp1, u32 isp2) {
    EdgeBetweenISPS ans;
    ans.index = nextInShortestPathBetweenISPS[isp1][isp2];
    ans.dist = shortestPathBetweenISPS[isp1][ans.index];
    return ans;
}


//Most likely won't be used
//SYN,SYNACK... List<Event>
struct PathBetweenISPS {
    u32 lengthOfTheEntirePath;
    u32 edgePop;
    EdgeBetweenISPS* edges;

    static void deInit(PathBetweenISPS &p) {
        free(p.edges);
        p.edges = nullptr;
    }

};
inline PathBetweenISPS getShortestPathBetweenISPS(u32 isp1, u32 isp2) {
    PathBetweenISPS ans;
    ans.lengthOfTheEntirePath = shortestPathBetweenISPS[isp1][isp2];


    u32 edgePop = 0;
    {
        u32 currentISP = isp1;
        while (currentISP != isp2) {
            currentISP = nextInShortestPathBetweenISPS[currentISP][isp2];
            ++edgePop;
        }
    }

    ans.edges = (EdgeBetweenISPS*) malloc(edgePop * sizeof(EdgeBetweenISPS));


    u32 currentISP = isp1;
    for (u32 currentIndex = 0; currentIndex < edgePop; ++currentIndex) {
        u32 nextISPInPath = nextInShortestPathBetweenISPS[currentISP][isp2];
        EdgeBetweenISPS nextEdge;
        nextEdge.index = nextISPInPath;
        nextEdge.dist = shortestPathBetweenISPS[currentISP][nextISPInPath];
        ans.edges[currentIndex] = nextEdge;

        currentISP = nextISPInPath;
    }

    return ans;
}

struct Circuit { //YOUR responsibility to free
    LocalNode** relaysArr;
    u32 size;
    
    template<usize N>
    static Circuit create(const LocalNode*(&arr)[N]) {
        Circuit ans;
        ans.size = (u32)N;

        ans.relaysArr = (LocalNode**)malloc(sizeof(LocalNode*) * ans.size);
        assertWithMessage(ans.relaysArr != nullptr, "Error, allocation failure");

        for (u32 i = 0; i < N; ++i) {
            ans.relaysArr[i] = arr[i];
        }
        return ans;
    }


    static Circuit unifyCircuits(Circuit c1, Circuit c2) { //Creates new circuit
        Circuit ans;
        ans.size = c1.size + c2.size;
        ans.relaysArr = (LocalNode**)malloc(sizeof(LocalNode*) * ans.size);
        assertWithMessage(ans.relaysArr != nullptr, "Error, allocation failure");

        for (u32 i = 0; i < c1.size; ++i) {
            ans.relaysArr[i] = c1.relaysArr[i];
        }
        for (u32 i = 0; i < c2.size; ++i) {
            ans.relaysArr[i + c1.size] = c2.relaysArr[i];
        }
        return ans;
    }

    static void destroy(Circuit c) {
        free(c.relaysArr);
    }
};
inline constexpr f32 OneMiB=8388608;
// Default throughput for each LocalNodeTag
constexpr std::array<f32, (u32)LocalNodeTag::_COUNT> tagThroughputDefaults = {
    OneMiB*2, // EntryRelay
    OneMiB*2, // MiddleRelay
    OneMiB*2, // ExitRelay
    OneMiB*2, // Client
    OneMiB*2, // Server
    OneMiB*2  // AnonymousService
};

// Current working throughputs — can be overridden by CLI
inline std::array<f32, (u32)LocalNodeTag::_COUNT> tagThroughputs = tagThroughputDefaults;

// Map CLI key names → LocalNodeTag enum values
inline const std::unordered_map<std::string, LocalNodeTag> throughputMap = {
    {"throughputEntryRelay",        LocalNodeTag::EntryRelay},
    {"throughputMiddleRelay",       LocalNodeTag::MiddleRelay},
    {"throughputExitRelay",         LocalNodeTag::ExitRelay},
    {"throughputClient",            LocalNodeTag::Client},
    {"throughputServer",            LocalNodeTag::Server},
    {"throughputAnonymousService",  LocalNodeTag::AnonymousService},
};

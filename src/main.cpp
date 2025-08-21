#include <iostream>
#include "mainStructs.h"
#include "eventSystem.h"
#include <cassert>
#include <string>
#include <fstream>
#include "RNG.h"
#include <sstream>
#include <random>
#include <tuple>
#include <ctime>    // for time()
#include"randomEvents.h"

// Convert uint32_t to dotted IP string

int randomValue(int from,int to)
{

    // Generate a random number between 8 and 16 (inclusive)
    int randomValue = from + std::rand() % (from - to + 1);
    return randomValue;
}
void printUsageAndExit() {
    std::cerr << "\n=== Simulator Usage ===\n";
    std::cerr << "Required arguments:\n";
    std::cerr << "  clients=<int>                (max " << MAX_CLIENT_POP << ")\n";
    std::cerr << "  anonymousServices=<int>      (max " << MAX_ANONYMOUS_SERVICES_POP << ")\n";
    std::cerr << "  entryRelays=<int>            (max " << MAX_ENTRY_RELAYS_POP << ")\n";
    std::cerr << "  middleRelays=<int>           (max " << MAX_MIDDLE_RELAYS_POP << ")\n";
    std::cerr << "  exitRelays=<int>             (max " << MAX_EXIT_RELAYS_POP << ")\n";
    std::cerr << "  servers=<int>                (max " << MAX_SERVER_POP << ")\n";
    std::cerr << "  attackType=<global|relay>\n\n";

    std::cerr << "Optional arguments:\n";
    std::cerr << "  userRequestProbability=<float in [0,1]>\n";
    std::cerr << "  probabilityToMakeRequestToAnAnonymousService=<float in [0,1]>\n";
    std::cerr << "  useDistanceOverhead=<0|1>\n";
    std::cerr << "  constantDistanceOverhead=<float >= 0>\n";
    std::cerr << "  makeASingleRequest=<0|1>\n";
    std::cerr << "  useWeighted=<0|1>    (uniform/Zipf-like RNG selection)\n\n";

    std::cerr << "Optional throughput overrides (default = 2000.0):\n";
    std::cerr << "  throughputEntryRelay=<float>\n";
    std::cerr << "  throughputMiddleRelay=<float>\n";
    std::cerr << "  throughputExitRelay=<float>\n";
    std::cerr << "  throughputClient=<float>\n";
    std::cerr << "  throughputServer=<float>\n";
    std::cerr << "  throughputAnonymousService=<float>\n\n";

    std::cerr << "Example:\n";
    std::cerr << "  ./simulator clients=1000 anonymousServices=500 entryRelays=400 middleRelays=400 exitRelays=400 servers=200 attackType=relay \\\n";
    std::cerr << "              userRequestProbability=0.05 probabilityToMakeRequestToAnAnonymousService=0.5 makeASingleRequest=1 useWeighted=1\n\n";
    std::exit(1);
}

// Build a descriptive CSV filename from current settings + timestamp
std::string buildCsvLogName(
    const localNodeTagDistribution& pops,
    AttackType attack,
    double userReqProb,
    bool weighted,       // true = Zipf, false = Uniform
    float constOverhead, // constant distance overhead
    bool singleRequest   // make a single request or not
) {
    std::ostringstream os;
    // timestamp
    const std::time_t t = std::time(nullptr);
    std::tm tm{};
#if defined(_WIN32)
    localtime_s(&tm, &t);
#else
    localtime_r(&t, &tm);
#endif

    os << "attack-" << attackTypeToString(attack)
       << "_cli-"  << pops.clientsPop
       << "_srv-"  << pops.serversPop
       << "_as-"   << pops.anonymousServicesPop
       << std::fixed << std::setprecision(3)
       << "_urp-"  << userReqProb
       << "_dist-" << (weighted ? "zipf" : "uniform")
       << std::setprecision(2)
       << "_const-" << constOverhead
       << "_single-" << (singleRequest ? 1 : 0)
       << "_ts-" << std::put_time(&tm, "%Y%m%d-%H%M%S")
       << ".csv";

    return os.str();
}


// Helper function to parse int from string safely
int parseInt(const std::string& valueStr) {
    try {
        return std::stoi(valueStr);
    } catch (const std::invalid_argument&) {
        std::cerr << "Invalid number format: " << valueStr << "\n";
        std::exit(EXIT_FAILURE);
    }
}
// Keep your original max check
inline void checkPopulationMax(const std::string& key, u32 value) {
    if (key == "clients")
        assertWithMessage(value <= MAX_CLIENT_POP, "Population exceeds maximum for: " + key);
    else if (key == "anonymousServices")
        assertWithMessage(value <= MAX_ANONYMOUS_SERVICES_POP, "Population exceeds maximum for: " + key);
    else if (key == "entryRelays")
        assertWithMessage(value <= MAX_ENTRY_RELAYS_POP, "Population exceeds maximum for: " + key);
    else if (key == "middleRelays")
        assertWithMessage(value <= MAX_MIDDLE_RELAYS_POP, "Population exceeds maximum for: " + key);
    else if (key == "exitRelays")
        assertWithMessage(value <= MAX_EXIT_RELAYS_POP, "Population exceeds maximum for: " + key);
    else if (key == "servers")
        assertWithMessage(value <= MAX_SERVER_POP, "Population exceeds maximum for: " + key);
}
void parseAndSetNodePopulations(int argc, char* argv[]) {
    if (argc == 1 || (argc > 1 && std::string(argv[1]) == "--help")) {
        printUsageAndExit();
    }

    auto& pops = allLocalNodes::allNodePops;

    // Map input keys → population fields
    std::unordered_map<std::string, u32*> variableMap;
    variableMap["clients"]           = &pops.clientsPop;
    variableMap["anonymousServices"] = &pops.anonymousServicesPop;
    variableMap["entryRelays"]       = &pops.entryRelaysPop;
    variableMap["middleRelays"]      = &pops.middleRelaysPop;
    variableMap["exitRelays"]        = &pops.exitRelaysPop;
    variableMap["servers"]           = &pops.serversPop;

    bool attackTypeSet = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto pos = arg.find('=');
        if (pos == std::string::npos) {
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY) {
                std::cerr << "Invalid argument: " << arg << ". Use format key=value.\n";
            }
            std::exit(1);
        }

        std::string key = arg.substr(0, pos);
        std::string valueStr = arg.substr(pos + 1);

        // Population variables
        if (auto it = variableMap.find(key); it != variableMap.end()) {
            int value = std::stoi(valueStr);
            assertWithMessage(value > 0, "Population must be greater than 0 for: " + key);
            checkPopulationMax(key, (u32)value);
            *it->second = (u32)value;
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] Set " << key << " = " << value << "\n";
        }
        // Attack type
        else if (key == "attackType") {
            if (valueStr == "global") {
                INTERSECTION_ATTACK::attack = GENERAL_GLOBAL_ADVERSARY_INTERSECTION_ATTACK;
                if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY) std::cout << "[INFO] Set attack type to GLOBAL\n";
            } else if (valueStr == "relay") {
                INTERSECTION_ATTACK::attack = PARTIAL_ADVERSARY_RELAY_LEVEL_INTERSECTION_ATTACK;
                if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY) std::cout << "[INFO] Set attack type to RELAY-LEVEL\n";
            } else {
                std::cerr << "Unknown attack type: " << valueStr << " (use 'global' or 'relay')\n";
                std::exit(1);
            }
            attackTypeSet = true;
        }
        // Simulation behavior parameters
        else if (key == "userRequestProbability") {
            double prob = std::stod(valueStr);
            assertWithMessage(prob >= 0.0 && prob <= 1.0, "userRequestProbability must be in [0, 1]");
            userRequestProbability = prob;
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] Set userRequestProbability = " << userRequestProbability << "\n";
        }
        else if (key == "probabilityToMakeRequestToAnAnonymousService") {
            double prob = std::stod(valueStr);
            assertWithMessage(prob >= 0.0 && prob <= 1.0, "probabilityToMakeRequestToAnAnonymousService must be in [0, 1]");
            probabilityToMakeRequestToAnAnonymousService = prob;
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] Set probabilityToMakeRequestToAnAnonymousService = " << prob << "\n";
        }
        // Optional distance overhead toggles
        else if (key == "useDistanceOverhead") {
            int flag = std::stoi(valueStr);
            USE_DISTANCE_OVERHEAD_WHEN_SENDING_PACKET_WITH_ISP = (flag != 0);
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] USE_DISTANCE_OVERHEAD_WHEN_SENDING_PACKET_WITH_ISP = "
                          << (USE_DISTANCE_OVERHEAD_WHEN_SENDING_PACKET_WITH_ISP ? "true" : "false") << "\n";
        }
        else if (key == "constantDistanceOverhead") {
            float value = std::stof(valueStr);
            assertWithMessage(value >= 0.0f, "CONSTANT_OVERHEAD_WHEN_SENDING_A_PACKET_WITH_ISP must be non-negative");
            CONSTANT_OVERHEAD_WHEN_SENDING_A_PACKET_WITH_ISP = value;
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] CONSTANT_OVERHEAD_WHEN_SENDING_A_PACKET_WITH_ISP = " << value << "\n";
        }
        // Make a single request (new)
        else if (key == "makeASingleRequest") {
            int flag = std::stoi(valueStr);
            makeASingleRequest = (flag != 0);
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] makeASingleRequest = " << (makeASingleRequest ? "true" : "false") << "\n";
        }
        // Use weighted/Zipf RNG (new)
        else if (key == "useWeighted") {
            int flag = std::stoi(valueStr);
            RNG::useWeighted = (flag != 0);
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] RNG::useWeighted = " << (RNG::useWeighted ? "true" : "false") << "\n";
        }
        // Throughput overrides
        else if (auto t = throughputMap.find(key); t != throughputMap.end()) {
            float value = std::stof(valueStr);
            assertWithMessage(value > 0.0f, "Throughput must be greater than 0 for: " + key);
            tagThroughputs[(u32)t->second] = value;
            if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
                std::cout << "[INFO] Set " << key << " = " << value << "\n";
        }
        else {
            std::cerr << "Unknown argument key: " << key << "\n";
            std::exit(1);
        }
    }

    // Make sure all required populations are set
    for (const auto& [name, ptr] : variableMap) {
        assertWithMessage(*ptr > 0, "Missing or zero population for: " + name);
    }
    assertWithMessage(attackTypeSet, "Missing required argument: attackType");

    // Compute total
    allLocalNodes::totalLocalNodesPop =
        pops.clientsPop +
        pops.anonymousServicesPop +
        pops.entryRelaysPop +
        pops.middleRelaysPop +
        pops.exitRelaysPop +
        pops.serversPop;

    if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
        std::cout << "[INFO] Total local nodes: " << allLocalNodes::totalLocalNodesPop << "\n";

    // Generate CSV log name
   const std::string csvName = buildCsvLogName(
    pops,
    INTERSECTION_ATTACK::attack,
    userRequestProbability,
    RNG::useWeighted,
    CONSTANT_OVERHEAD_WHEN_SENDING_A_PACKET_WITH_ISP,
    makeASingleRequest
);

    if (!PRINT_SCIENTIFIC_CSV_DATA_ONLY)
        std::cout << "[INFO] CSV log file will be: " << csvName << "\n";

    ATTACK_LOGGER::open(csvName);
}




void initializeEnvironment()
{
   
    //Not particularly concerned about the performance of a lot of this function, because calculating the shortest paths will take the overwhelming bulk of it, just don't write it like trash

     //  if(!PRINT_SCIENTIFIC_CSV_DATA_ONLY){
    std::cout<<"user request probability:"<<userRequestProbability<<std::endl;
    std::cout<<"Zig distribution:"<<RNG::useWeighted<<std::endl;
    std::cout<<"attack type:"<<INTERSECTION_ATTACK::attack<<std::endl;
    std::cout<<"Simulate user behavior:"<<makeASingleRequest<<std::endl;
    std::cout<<"clientsPop"<<allLocalNodes::allNodePops.clientsPop<<"\n";
    std::cout<<"anonymousServicesPop"<<allLocalNodes::allNodePops.anonymousServicesPop<<"\n";
    std::cout<<"entryRelaysPop"<<allLocalNodes::allNodePops.entryRelaysPop<<"\n";
    std::cout<<"middleRelaysPop"<<allLocalNodes::allNodePops.middleRelaysPop<<"\n";
    std::cout<<"exitRelaysPop"<<allLocalNodes::allNodePops.exitRelaysPop<<"\n";
    std::cout<<"serversPop"<<allLocalNodes::allNodePops.serversPop<<"\n";
       //     }
    popISP = 10;
    RNG::initializeDistributions();

    //First step: Distribute local node tags.
    {

        u32* distributions[LocalNodeTagPop]; //For each localNodeTag, for each ISP how many it has

        for (u32 i = 0; i < LocalNodeTagPop; ++i) {
            distributions[i] = RNG::generateRandomIntsWithSpecificSum(popISP, allLocalNodes::allNodePops.arr[i]);
        }
        defer({
            for (u32 i = 0; i < LocalNodeTagPop; ++i) {
                free(distributions[i]);
            }
        });


        //A bit of drudgework to increment the arrays that store all localNodes
        //std::vector instead of global arrays would skip this, but this approach gives more performance and control
        localNodeTagDistribution nodesInsertedSoFar;
        for (u32 i =0; i < LocalNodeTagPop; ++i) {
            nodesInsertedSoFar.arr[i] = 0;
        }


        //Calculate the IP of each local node and allocate memory for its children
        const u32 jump = 1 << MAX_DEGREE_OF_FREEDOM; //    0000-0000 0000-0001 0000-0000 0000-0000

        //Nikolas: Ara edo kaneis calculate posa localnodes exei o kathe ISP
        u32 currentIp = jump;
        for (u32 i = 0; i<popISP; ++i, currentIp += jump ) {

            u32 clientPop = 0;
            for (u32 j = 0; j < LocalNodeTagPop; ++j) {
                clientPop += distributions[j][i];
            }
            arrayISP[i].clientPop = clientPop;

            arrayISP[i].myClients = (LocalNode*)malloc(clientPop * sizeof(LocalNode));
            assertWithMessage(arrayISP[i].myClients != nullptr, "error, allocation failure");
            //never free this because it lasts the lifetime of the program.

            //Generate the ip adress for current ISP
            arrayISP[i].baseIp.rawNumber = currentIp;
            arrayISP[i].numberOfFixedBitsInIP = (u8)(32 - randomValue(MIN_DEGREE_OF_FREEDOM,MAX_DEGREE_OF_FREEDOM));
            while ( (1 << (32 - arrayISP[i].numberOfFixedBitsInIP)) - 1 < clientPop) {
                --arrayISP[i].numberOfFixedBitsInIP;
                //Reduce number of fixed bits if they are not enough for our clients
                //We don't have to worry about exceeding maxdegree of freedom because it's a very large value
                //While loop not optimal but more than good enough
            }
        }
        //Calculate the child of each localNode

        for (u32 i = 0; i<popISP; ++i) {

            u32 currentChildIndex = 0;
            for (u8 currentTag = 0; currentTag < LocalNodeTagPop; ++currentTag) {//Nicolas : This is for iterating all the different types that exist
                for (u32 j = 0; j < distributions[currentTag][i]; ++j, ++currentChildIndex) {//Nicolas: This

                    LocalNode tmp = LocalNode {
                        .myISP = i,
                        .throughput = tagThroughputs[(u32)currentTag], // Improve this in the future
                        .myIP = {
                            .rawNumber = arrayISP[i].baseIp.rawNumber + currentChildIndex + 1
                        },
                        .type = (LocalNodeTag)currentTag,
                        .nextAvailableMoment = 0 //All nodes start available
                        
                    };


                    //The node is completed, now we need to add that node to the ISP and the AllLocalNodes global data
                    arrayISP[i].myClients[currentChildIndex] = tmp;
                    allLocalNodes::as2DArray[currentTag][nodesInsertedSoFar.arr[currentTag]] = &arrayISP[i].myClients[currentChildIndex];
                    ++nodesInsertedSoFar.arr[currentTag];
                }
            }
            //Sanity check
            assertWithMessage(currentChildIndex == arrayISP[i].clientPop, "YOU HAVE A BUG, ISP hasn't initialized all it's children");
        }

        //Sanity check
        for (u32 i=0; i<LocalNodeTagPop; ++i) {
            assertWithMessage(nodesInsertedSoFar.arr[i] == allLocalNodes::allNodePops.arr[i], "YOU HAVE A BUG, nodes of a tag inserted are not the same as provided by the user");
        }

    }
    //TODO: TO PUT BETTER RANDOM CHOOSE
    for (usize i = 0; i < MAX_CLIENT_POP; ++i) {
            u32 entryIndex = RNG::getRandomIndexForTag(LocalNodeTag::EntryRelay);
            entryGuardsOfClients[i]=allLocalNodes::entryRelays[entryIndex];        
    }
    //Initialize anonymous services relays, this is an extremely lazy but good-enough implementation
    for (usize i = 0; i < allLocalNodes::allNodePops.anonymousServicesPop; ++i) {
        if (doNotUseEntryGuard)
        {
            auto [i0, i1, i2] = RNG::generate3RandomInts(0, allLocalNodes::allNodePops.entryRelaysPop);
            anonymousServicesIntroductionPoints[i][0].EntryRelay = allLocalNodes::entryRelays[i0];
            anonymousServicesIntroductionPoints[i][1].EntryRelay = allLocalNodes::entryRelays[i1];
            anonymousServicesIntroductionPoints[i][2].EntryRelay = allLocalNodes::entryRelays[i2];
        }
        else
        {
            u32 i0 = rand() %allLocalNodes::allNodePops.entryRelaysPop;
            anonymousServicesIntroductionPoints[i][0].EntryRelay = allLocalNodes::entryRelays[i0];
            anonymousServicesIntroductionPoints[i][1].EntryRelay = allLocalNodes::entryRelays[i0];
            anonymousServicesIntroductionPoints[i][2].EntryRelay = allLocalNodes::entryRelays[i0];
        }

        auto [m0, m1, m2] = RNG::generate3RandomInts(0, allLocalNodes::allNodePops.middleRelaysPop/2);
        auto [m3, m4, m5] = RNG::generate3RandomInts(allLocalNodes::allNodePops.middleRelaysPop/2, allLocalNodes::allNodePops.middleRelaysPop);

        anonymousServicesIntroductionPoints[i][0].MiddleRelay1 = allLocalNodes::middleRelays[m0];
        anonymousServicesIntroductionPoints[i][0].MiddleRelay2 = allLocalNodes::middleRelays[m1];
        anonymousServicesIntroductionPoints[i][1].MiddleRelay1 = allLocalNodes::middleRelays[m2];
        anonymousServicesIntroductionPoints[i][1].MiddleRelay2 = allLocalNodes::middleRelays[m3];
        anonymousServicesIntroductionPoints[i][2].MiddleRelay1 = allLocalNodes::middleRelays[m4];
        anonymousServicesIntroductionPoints[i][2].MiddleRelay2 = allLocalNodes::middleRelays[m5];
    }
    

    //Initialize 2D arrays with distance and shortest path as infinity
    for (usize i = 0; i < popISP; ++i) {
        for (usize j = 0; j < popISP; ++j) {
            neighboringDistanceBetweenISPS[i][j] = INF;
            shortestPathBetweenISPS[i][j] = INF;
            nextInShortestPathBetweenISPS[i][j] = INF; //Unnecessary line because they will all be initialized, but may help debugging
            if (i == j) 
            {
                neighboringDistanceBetweenISPS[i][j] = 0;
                shortestPathBetweenISPS[i][j] = 0;
                nextInShortestPathBetweenISPS[i][j] = i;
            }
        }
    }

    //Connect the isps function
    const auto connect = [](u32 node1, u32 node2, u32 dist) {
        EdgeBetweenISPS e1, e2;
        e1.index = node2;
        e1.dist = dist;
        e2.index = node1;
        e2.dist = dist;
        edgesBetweenISPS[node1].push_back(e1);
        edgesBetweenISPS[node2].push_back(e2);

        neighboringDistanceBetweenISPS[node1][node2] = dist;
        neighboringDistanceBetweenISPS[node2][node1] = dist;
        
        shortestPathBetweenISPS[node1][node2] = dist;
        shortestPathBetweenISPS[node2][node1] = dist;

        nextInShortestPathBetweenISPS[node1][node2] = node2;
        nextInShortestPathBetweenISPS[node2][node1] = node1;
    };


    //Construct graph between ISPs
    for (u32 i=0; i<popISP-1; ++i) { //-1 because we only connect to ones that have a higher index than we do
        connect(i, i+1, (u32)randomValue(1,5));//We connect everything with the next neighbor to be sure that is fully conected
        
        u32 currentJump = i + 1;
        while (true) {
            currentJump += randomValue(1, popISP/2);//we  connect until we get out of bounds
            if (currentJump >= popISP) {
                break; 
            }
            connect(i, currentJump, (u32)randomValue(1,5));
        }
    }


    //Floyd - Warshall algorith to calculate shortest paths
    for (u32 a = 0; a < popISP; ++a) {
        for (u32 b = 0; b < popISP; ++b) {
            for (u32 c = 0; c < popISP; ++c) {
        
            if  (shortestPathBetweenISPS[b][c] > shortestPathBetweenISPS[b][a] + shortestPathBetweenISPS[a][c]) {
                    shortestPathBetweenISPS[b][c] = shortestPathBetweenISPS[b][a] + shortestPathBetweenISPS[a][c];
                    nextInShortestPathBetweenISPS[b][c] = a;
                }
            }
        }
    }

    INTERSECTION_ATTACK::initializeExitRelayMap();
}



int main(int argc, char* argv[]) {
/*    int num_sessions = 1000;
    for (int i = 0; i < num_sessions; ++i) {
        auto s = UserSessionSimulator::simulateSession();
        std::cout << "Session " << i+1 << ": " << s.pages << " pages\n";
        std::cout << "  Dwell times (s): ";
        for (double t : s.dwell_times) {
            std::cout << t << " ";
        }
        std::cout << "\n  Total duration: " << s.total_duration << " s\n\n";
    }
    return 0;
*/
    parseAndSetNodePopulations(argc, argv);
    initializeEnvironment();

    EventSystemFunctionData uninitializedData;
    int maximum_number_of_epochs=1000000;
   // userRequestProbability=0.09;
    probabilityToMakeRequestToAnAnonymousService=0.5;
  
    for (int i = 0; i < maximum_number_of_epochs; ++i)
    {
        f32 floatForm = (f32) i;
        eventSystem::addEvent(floatForm, eventSystem::getNextID(), uninitializedData, randomEventsFunction);
    }


    while(eventSystem::isNotEmpty()) {
        eventSystem::runNextEvent();
    }
}
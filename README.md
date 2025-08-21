## 4.1 Simulator High-Level Overview

The simulator employs a **fully event-driven architecture** in which all protocol operations — from circuit construction to packet transmission — are decomposed into **atomic events**.

- All events are stored in a **global priority queue**, ordered by execution time.  
- The simulator repeatedly extracts the earliest event, executes it, and inserts any follow-up events into the queue.  
- This design enforces **causal ordering**, avoids race conditions, and scales to thousands of nodes **without explicit threading**.  
- **Postponement** is handled by updating the event’s execution time in the queue instead of generating new events.  

---

### Primary Event Types

1. **Random Event Generator**  
   - Triggered at every integer time unit (`1, 2, 3, …`).  
   - Controlled by `userRequestProbability`.  
   - If a request is issued:  
     - With probability `0.5`:  
       - The client generates a **clearnet request** through a **three-hop circuit** (entry → middle → exit relay).  
       - Response is returned through the same path.  
       - Targets selected via **Zipf** or **uniform distribution**.  
     - Otherwise:  
       - The client chooses an **anonymous service**, selects an introduction point, executes the **rendezvous protocol**, then exchanges packets.  

2. **Local-to-ISP Transmission**  
   - Transfers a packet from a local node (client, relay, server, or anonymous service) to its ISP.  
   - If the node is busy, the event is postponed.  
   - Next available time:  
     ```
     t_next = t_current + (packet_size / throughput)
     ```

3. **ISP-to-ISP Transmission**  
   - Packet traverses inter-ISP hops until destination ISP is reached.  
   - Shortest paths precomputed with **Floyd–Warshall**.  
   - Each hop incurs either:  
     - Constant latency, or  
     - Distance-proportional delay.  
   - If destination ISP reached → event becomes ISP-to-local-node delivery.  

4. **ISP-to-Local-Node Delivery**  
   - Final delivery step from ISP to node.  
   - If the node is busy, execution is rescheduled.  
   - Updates node’s availability (`t_next = t_current + (packet_size / throughput)`).  
   - May trigger the next event in the **eventchain**.  

> **Note:** All packets are fixed at **512 bytes**.

---

### Eventchain Abstraction

- An **eventchain** is an ordered list of events representing end-to-end packet processing.  
- Each event, once executed, schedules the next event in the global queue.  
- Ensures **step-by-step propagation** while respecting throughput and ISP constraints.  

---

### Example Flow: Client → Server (3-hop circuit)

```mermaid
flowchart TD
    A[Client] -->|LocalToIsp| B[Client ISP]
    B -->|IspToIsp δ1| C[Entry Relay ISP]
    C -->|IspToLocal δ2| D[Entry Relay]
    D -->|LocalToIsp δ3| E[Entry Relay ISP]
    E -->|IspToIsp| F[Middle Relay ISP]
    F -->|IspToLocal| G[Middle Relay]
    G -->|LocalToIsp| H[Middle Relay ISP]
    H -->|IspToIsp| I[Exit Relay ISP]
    I -->|IspToLocal| J[Exit Relay]
    J -->|LocalToIsp| K[Exit Relay ISP]
    K -->|IspToIsp| L[Server ISP]
    L -->|IspToLocal| M[Server]

=== Simulator Usage ===

Required arguments:
  clients=<int>                (max MAX_CLIENT_POP)
  anonymousServices=<int>      (max MAX_ANONYMOUS_SERVICES_POP)
  entryRelays=<int>            (max MAX_ENTRY_RELAYS_POP)
  middleRelays=<int>           (max MAX_MIDDLE_RELAYS_POP)
  exitRelays=<int>             (max MAX_EXIT_RELAYS_POP)
  servers=<int>                (max MAX_SERVER_POP)
  attackType=<global|relay>

Optional arguments:
  userRequestProbability=<float in [0,1]>
  probabilityToMakeRequestToAnAnonymousService=<float in [0,1]>
  useDistanceOverhead=<0|1>
  constantDistanceOverhead=<float >= 0>
  makeASingleRequest=<0|1>
  useWeighted=<0|1>    (uniform/Zipf-like RNG selection)

Optional throughput overrides (default = 2000.0):
  throughputEntryRelay=<float>
  throughputMiddleRelay=<float>
  throughputExitRelay=<float>
  throughputClient=<float>
  throughputServer=<float>
  throughputAnonymousService=<float>

Example:
  ./simulator clients=1000 anonymousServices=500 entryRelays=400 middleRelays=400 exitRelays=400 servers=200 attackType=relay \
              userRequestProbability=0.05 probabilityToMakeRequestToAnAnonymousService=0.5 makeASingleRequest=1 useWeighted=1

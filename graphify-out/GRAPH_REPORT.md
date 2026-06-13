# Graph Report - PakanIkanPLT  (2026-06-13)

## Corpus Check
- 12 files · ~1,448 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 48 nodes · 50 edges · 7 communities (6 shown, 1 thin omitted)
- Extraction: 86% EXTRACTED · 14% INFERRED · 0% AMBIGUOUS · INFERRED: 7 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `12e5351e`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- [[_COMMUNITY_Community 0|Community 0]]
- [[_COMMUNITY_Community 1|Community 1]]
- [[_COMMUNITY_Community 2|Community 2]]
- [[_COMMUNITY_Community 3|Community 3]]
- [[_COMMUNITY_Community 4|Community 4]]
- [[_COMMUNITY_Community 5|Community 5]]

## God Nodes (most connected - your core abstractions)
1. `Conversation Resume: Pakan Ikan Otomatis PlatformIO Migration` - 6 edges
2. `Pakan Ikan Otomatis (Automatic Fish Feeder)` - 6 edges
3. `Project Resume: Pakan Ikan Otomatis (Automatic Fish Feeder)` - 6 edges
4. `loop()` - 5 edges
5. `setup()` - 4 edges
6. `🛠️ Technical State` - 4 edges
7. `updateDisplay()` - 3 edges
8. `getCurrentTime()` - 3 edges
9. `⚙️ Calibration & Settings` - 3 edges
10. `initDisplay()` - 2 edges

## Surprising Connections (you probably didn't know these)
- `loop()` --calls--> `startFeeding()`  [INFERRED]
  D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/main.cpp → D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/feeding.cpp
- `loop()` --calls--> `updateFeeding()`  [INFERRED]
  D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/main.cpp → D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/feeding.cpp
- `setup()` --calls--> `initDisplay()`  [INFERRED]
  D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/main.cpp → D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/display.cpp
- `loop()` --calls--> `updateDisplay()`  [INFERRED]
  D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/main.cpp → D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/display.cpp
- `setup()` --calls--> `initFeeding()`  [INFERRED]
  D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/main.cpp → D:/MyCode/Pakan-Ikan-Otomatis-Arduino/PlatformIO_Project/src/feeding.cpp

## Import Cycles
- None detected.

## Communities (7 total, 1 thin omitted)

### Community 0 - "Community 0"
Cohesion: 0.17
Nodes (11): ⚙️ Calibration & Settings, Conversation Resume: Pakan Ikan Otomatis PlatformIO Migration, Feeding Schedules, Hardware Configuration, Key Improvements Made, ⚠️ Known Issues & Notes, 🚀 Next Steps for Resuming, 📌 Project Overview (+3 more)

### Community 1 - "Community 1"
Cohesion: 0.31
Nodes (4): initDisplay(), initFeeding(), setup(), initRTC()

### Community 2 - "Community 2"
Cohesion: 0.25
Nodes (7): TimeData, TimeData, updateDisplay(), startFeeding(), updateFeeding(), loop(), getCurrentTime()

### Community 3 - "Community 3"
Cohesion: 0.29
Nodes (6): 🔧 Calibration, 🚀 Features, 🛠️ Hardware Requirements, ⚙️ Installation & Setup, Pakan Ikan Otomatis (Automatic Fish Feeder), 📂 Project Structure (PlatformIO)

### Community 4 - "Community 4"
Cohesion: 0.29
Nodes (6): Hardware Configuration, Key Features, Project Overview, Project Resume: Pakan Ikan Otomatis (Automatic Fish Feeder), Software Architecture (Modular), Technical Specifications

## Knowledge Gaps
- **22 isolated node(s):** `recommendations`, `unwantedRecommendations`, `TimeData`, `TimeData`, `📌 Project Overview` (+17 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **1 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `loop()` connect `Community 2` to `Community 1`?**
  _High betweenness centrality (0.049) - this node is a cross-community bridge._
- **Are the 4 inferred relationships involving `loop()` (e.g. with `updateDisplay()` and `startFeeding()`) actually correct?**
  _`loop()` has 4 INFERRED edges - model-reasoned connections that need verification._
- **Are the 3 inferred relationships involving `setup()` (e.g. with `initDisplay()` and `initFeeding()`) actually correct?**
  _`setup()` has 3 INFERRED edges - model-reasoned connections that need verification._
- **What connects `recommendations`, `unwantedRecommendations`, `TimeData` to the rest of the system?**
  _22 weakly-connected nodes found - possible documentation gaps or missing edges._
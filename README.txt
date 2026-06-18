====================================================================
  Heap Performance Analyser — Design and Analysis of Algorithms
  FAST NUCES — Department of Cyber Security
====================================================================

LANGUAGE
--------
C++17 (no external libraries — only the C++ standard library)

REQUIREMENTS
------------
A C++ compiler with C++17 support:
  Linux/Mac : g++ 7+ or clang++ 5+
  Windows   : MinGW-w64 / MSVC 2017+

PROJECT STRUCTURE
-----------------
  main.cpp              <- Entry point: compile and run this
  test_heaps.cpp        <- Correctness + performance test suite
  output_results.txt    <- Generated output table (after running main)
  README.txt            <- This file
  src/
    BinaryHeap.h        <- Array-based min-heap
    FibonacciHeap.h     <- Fibonacci heap with cascading cuts
    HollowHeap.h        <- Hollow heap with lazy deletion
    Dijkstra.h          <- Generic Dijkstra (works with any heap)
    GraphLoader.h       <- Parses .road-d dataset files
  data/
    Hongkong.road-d     <- Small  dataset  (43,620 nodes,  91,542 edges)
    Shanghai.road-d     <- Medium dataset  (390,171 nodes, 855,982 edges)
    Chongqing.road-d    <- Large  dataset  (1,185,464 nodes, 2,428,866 edges)

HOW TO COMPILE
--------------

Linux / macOS:
  g++ -O2 -std=c++17 -o heap_benchmark main.cpp
  g++ -O2 -std=c++17 -o test_heaps     test_heaps.cpp

Windows (MinGW):
  g++ -O2 -std=c++17 -o heap_benchmark.exe main.cpp
  g++ -O2 -std=c++17 -o test_heaps.exe     test_heaps.cpp

HOW TO RUN
----------
1. Run the benchmark:
     ./heap_benchmark          (Linux/macOS)
     heap_benchmark.exe        (Windows)

2. You will be prompted to select a dataset:
     [1] Hong Kong (Small)
     [2] Shanghai (Medium)
     [3] Chongqing (Large)

3. The program will:
   - Load the selected graph
   - Run Dijkstra with Binary, Fibonacci, and Hollow heaps
   - Print a performance comparison table to the console
   - Save results to output_results.txt
   - Run Experiment B (100,000 random operations)

TO RUN TESTS:
     ./test_heaps              (Linux/macOS)
     test_heaps.exe            (Windows)

This verifies:
  - 8 correctness tests per heap (24 total)
  - Performance micro-benchmarks (n=50,000 ops)

NOTES
-----
- No external libraries are used. Only standard C++ headers:
  <vector>, <unordered_map>, <string>, <fstream>, <sstream>,
  <iostream>, <iomanip>, <chrono>, <random>, <algorithm>, <cmath>,
  <limits>, <stack>, <functional>
- All three heaps implement the same interface, ensuring fair comparison.
- The Dijkstra template in Dijkstra.h accepts any heap type.
- For Chongqing (large), runtime may be several minutes.

DATASET FORMAT
--------------
Lines starting with '#' are comments.
Data lines: u  v  distance(u,v)
Graph is treated as undirected (each edge stored in both directions).

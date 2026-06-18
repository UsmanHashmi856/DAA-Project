#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <sstream>
#include <functional>
#include <algorithm>

#include "src/GraphLoader.h"
#include "src/BinaryHeap.h"
#include "src/FibonacciHeap.h"
#include "src/HollowHeap.h"
#include "src/Dijkstra.h"

struct BenchmarkRow {
    std::string heapName;

    int nodes;
    int edges;

    double avgInsertUs;
    double avgExtractUs;
    double avgDecreaseUs;

    double totalMs;

    int height;
    int treeCount;

    double memoryMb;

    long long cascadingCuts;
};

struct ProfileRow {
    std::string heapName;

    long long inserts;
    long long extracts;
    long long decreases;

    double avgInsertUs;
    double avgExtractUs;
    double avgDecreaseUs;
};

struct DatasetInfo {
    std::string path;
    std::string label;
};

static std::vector<DatasetInfo> datasets = {
    {"data/Hongkong.road-d", "Hong Kong (Small)"},
    {"data/Shanghai.road-d", "Shanghai (Medium)"},
    {"data/Chongqing.road-d", "Chongqing (Large)"}
};

double estimateMemoryMb(
    int nodeCount,
    const std::string& heapName
) {
    int bytesPerNode = 32;

    if (heapName == "Fibonacci") {
        bytesPerNode = 64;
    } else if (heapName == "Hollow") {
        bytesPerNode = 48;
    }

    double totalBytes =
        static_cast<double>(nodeCount) *
        (bytesPerNode + 8 + 4 + 1);

    return totalBytes / (1024.0 * 1024.0);
}

std::string makeSeparator(
    int width,
    char symbol = '-'
) {
    return std::string(width, symbol);
}

std::string formatBenchmarkRow(
    const BenchmarkRow& row
) {
    std::ostringstream output;

    output << std::left
           << std::setw(12)
           << row.heapName

           << std::right
           << std::setw(10)
           << row.nodes

           << std::setw(12)
           << row.edges

           << std::fixed
           << std::setprecision(3)

           << std::setw(10)
           << row.avgInsertUs

           << std::setw(10)
           << row.avgExtractUs

           << std::setw(10)
           << row.avgDecreaseUs

           << std::setw(11)
           << std::setprecision(1)
           << row.totalMs

           << std::setw(8)
           << row.height

           << std::setw(8)
           << row.treeCount

           << std::setw(9)
           << std::setprecision(1)
           << row.memoryMb;

    if (row.cascadingCuts >= 0) {
        output << std::setw(11)
               << row.cascadingCuts;
    } else {
        output << std::setw(11)
               << "N/A";
    }

    return output.str();
}

std::string buildTableHeader() {
    std::ostringstream output;

    output << std::left
           << std::setw(12)
           << "Heap"

           << std::right
           << std::setw(10)
           << "Nodes"

           << std::setw(12)
           << "Edges"

           << std::setw(10)
           << "Ins(us)"

           << std::setw(10)
           << "Ext(us)"

           << std::setw(10)
           << "Dec(us)"

           << std::setw(11)
           << "Time(ms)"

           << std::setw(8)
           << "Height"

           << std::setw(8)
           << "#Trees"

           << std::setw(9)
           << "Mem(MB)"

           << std::setw(11)
           << "CascCuts";

    return output.str();
}

template <typename Heap>
ProfileRow runProfiling(
    const std::string& heapName,
    int operationCount = 100000
) {
    using Clock =
        std::chrono::high_resolution_clock;

    std::mt19937_64 rng(42);

    std::uniform_real_distribution<double>
        keyDistribution(0.0, 1e6);

    std::uniform_int_distribution<int>
        operationDistribution(0, 2);

    Heap heap;

    std::vector<int> activeIds;
    std::vector<double> activeKeys;

    double insertTime = 0;
    double extractTime = 0;
    double decreaseTime = 0;

    long long insertCount = 0;
    long long extractCount = 0;
    long long decreaseCount = 0;

    int currentId = 0;

    for (int i = 0; i < operationCount; ++i) {
        int operation =
            operationDistribution(rng);

        if (
            operation == 0 ||
            activeIds.size() < 10
        ) {
            double key =
                keyDistribution(rng);

            int id = currentId++;

            auto start = Clock::now();

            heap.insert(key, id);

            insertTime +=
                std::chrono::duration<double>(
                    Clock::now() - start
                ).count();

            activeIds.push_back(id);
            activeKeys.push_back(key);

            ++insertCount;
        }
        else if (
            operation == 1 &&
            !activeIds.empty()
        ) {
            auto start = Clock::now();

            auto [key, id] =
                heap.extractMin();

            extractTime +=
                std::chrono::duration<double>(
                    Clock::now() - start
                ).count();

            ++extractCount;

            if (id >= 0) {
                auto iterator =
                    std::find(
                        activeIds.begin(),
                        activeIds.end(),
                        id
                    );

                if (iterator != activeIds.end()) {
                    int index =
                        static_cast<int>(
                            iterator - activeIds.begin()
                        );

                    activeIds.erase(iterator);

                    activeKeys.erase(
                        activeKeys.begin() + index
                    );
                }
            }
        }
        else if (!activeIds.empty()) {
            std::uniform_int_distribution<int>
                pickIndex(
                    0,
                    static_cast<int>(
                        activeIds.size()
                    ) - 1
                );

            int index =
                pickIndex(rng);

            int id =
                activeIds[index];

            double newKey =
                activeKeys[index] *
                std::uniform_real_distribution<double>(
                    0,
                    1
                )(rng);

            auto start = Clock::now();

            heap.decreaseKey(id, newKey);

            decreaseTime +=
                std::chrono::duration<double>(
                    Clock::now() - start
                ).count();

            activeKeys[index] = newKey;

            ++decreaseCount;
        }
    }

    ProfileRow result;

    result.heapName = heapName;

    result.inserts = insertCount;
    result.extracts = extractCount;
    result.decreases = decreaseCount;

    result.avgInsertUs =
        insertCount > 0
            ? (insertTime / insertCount) * 1e6
            : 0;

    result.avgExtractUs =
        extractCount > 0
            ? (extractTime / extractCount) * 1e6
            : 0;

    result.avgDecreaseUs =
        decreaseCount > 0
            ? (decreaseTime / decreaseCount) * 1e6
            : 0;

    return result;
}

int main() {
    std::cout
        << "\n"
        << makeSeparator(68, '=')
        << "\n";

    std::cout
        << "  Heap Performance Analyzer -- Dijkstra Shortest Path\n";

    std::cout
        << makeSeparator(68, '=')
        << "\n\n";

    std::cout
        << "  Available datasets:\n";

    for (
        int i = 0;
        i < static_cast<int>(datasets.size());
        ++i
    ) {
        std::cout
            << "    ["
            << i + 1
            << "] "
            << datasets[i].label
            << "\n";
    }

    std::cout
        << "\n  Select dataset [1/2/3]: ";

    int choice = 0;

    while (
        !(std::cin >> choice) ||
        choice < 1 ||
        choice > 3
    ) {
        std::cin.clear();

        std::cin.ignore(10000, '\n');

        std::cout
            << "  Invalid input. Enter 1, 2, or 3: ";
    }

    const DatasetInfo& dataset =
        datasets[choice - 1];

    std::cout
        << "\n  Loading "
        << dataset.label
        << "...\n";

    Graph graph;

    try {
        graph = loadGraph(dataset.path);
    }
    catch (const std::exception& exception) {
        std::cerr
            << "  Error: "
            << exception.what()
            << "\n";

        return 1;
    }

    std::cout
        << "  Loaded "
        << graph.numNodes
        << " nodes and "
        << graph.numEdges
        << " edges.\n";

    std::cout
        << "\n  Running Dijkstra using all heap implementations...\n\n";

    std::vector<BenchmarkRow> benchmarkRows;

    auto runBenchmark =
        [&](const std::string& heapName,
            auto benchmarkFunction) {

        std::cout
            << "  ["
            << heapName
            << "] running...";

        std::cout.flush();

        DijkstraResult result =
            benchmarkFunction();

        std::cout
            << " done ("
            << std::fixed
            << std::setprecision(0)
            << result.totalTime * 1000
            << " ms)\n";

        BenchmarkRow row;

        row.heapName = heapName;
        row.nodes = graph.numNodes;
        row.edges = graph.numEdges;

        row.avgInsertUs =
            result.avgInsertUs;

        row.avgExtractUs =
            result.avgExtractUs;

        row.avgDecreaseUs =
            result.avgDecreaseUs;

        row.totalMs =
            result.totalTime * 1000;

        row.height =
            result.peakHeight;

        row.treeCount =
            result.peakTrees;

        row.memoryMb =
            estimateMemoryMb(
                graph.numNodes,
                heapName
            );

        row.cascadingCuts = -1;

        benchmarkRows.push_back(row);

        return result;
    };

    runBenchmark(
        "Binary",
        [&] {
            return dijkstra<BinaryHeap>(
                graph.adj,
                0,
                graph.numNodes
            );
        }
    );

    runBenchmark(
        "Fibonacci",
        [&] {
            return dijkstra<FibonacciHeap>(
                graph.adj,
                0,
                graph.numNodes
            );
        }
    );

    runBenchmark(
        "Hollow",
        [&] {
            return dijkstra<HollowHeap>(
                graph.adj,
                0,
                graph.numNodes
            );
        }
    );

    std::string divider =
        makeSeparator(100);

    std::string tableHeader =
        buildTableHeader();

    auto printExperimentA =
        [&](std::ostream& output) {

        output
            << "\n"
            << divider
            << "\n";

        output
            << "  EXPERIMENT A -- Static Routing\n";

        output
            << "  Dataset: "
            << dataset.label
            << " | Nodes: "
            << graph.numNodes
            << " | Edges: "
            << graph.numEdges
            << "\n";

        output
            << divider
            << "\n";

        output
            << tableHeader
            << "\n";

        output
            << divider
            << "\n";

        for (const auto& row : benchmarkRows) {
            output
                << formatBenchmarkRow(row)
                << "\n";
        }

        output
            << divider
            << "\n";
    };

    printExperimentA(std::cout);

    std::cout
        << "\n  Running Experiment B -- 100,000 random operations...\n\n";

    std::vector<ProfileRow> profileRows;

    auto runProfile =
        [&](const std::string& heapName,
            auto profileFunction) {

        std::cout
            << "  ["
            << heapName
            << "] profiling...";

        std::cout.flush();

        ProfileRow row =
            profileFunction();

        profileRows.push_back(row);

        std::cout
            << " done\n";
    };

    runProfile(
        "Binary",
        [&] {
            return runProfiling<BinaryHeap>(
                "Binary"
            );
        }
    );

    runProfile(
        "Fibonacci",
        [&] {
            return runProfiling<FibonacciHeap>(
                "Fibonacci"
            );
        }
    );

    runProfile(
        "Hollow",
        [&] {
            return runProfiling<HollowHeap>(
                "Hollow"
            );
        }
    );

    auto printExperimentB =
        [&](std::ostream& output) {

        std::string dividerLine =
            makeSeparator(80);

        output
            << "\n"
            << dividerLine
            << "\n";

        output
            << "  EXPERIMENT B -- Operation Profiling (100,000 random ops)\n";

        output
            << dividerLine
            << "\n";

        output
            << std::left
            << std::setw(12)
            << "Heap"

            << std::right
            << std::setw(10)
            << "#Inserts"

            << std::setw(11)
            << "#Extracts"

            << std::setw(12)
            << "#Decreases"

            << std::setw(10)
            << "Ins(us)"

            << std::setw(10)
            << "Ext(us)"

            << std::setw(10)
            << "Dec(us)"
            << "\n";

        output
            << dividerLine
            << "\n";

        for (const auto& row : profileRows) {
            output
                << std::left
                << std::setw(12)
                << row.heapName

                << std::right
                << std::setw(10)
                << row.inserts

                << std::setw(11)
                << row.extracts

                << std::setw(12)
                << row.decreases

                << std::fixed
                << std::setprecision(3)

                << std::setw(10)
                << row.avgInsertUs

                << std::setw(10)
                << row.avgExtractUs

                << std::setw(10)
                << row.avgDecreaseUs
                << "\n";
        }

        output
            << dividerLine
            << "\n";
    };

    printExperimentB(std::cout);

    std::ofstream outputFile(
        "output_results.txt"
    );

    if (outputFile.is_open()) {
        outputFile
            << makeSeparator(100, '=')
            << "\n";

        outputFile
            << "  Performance Analysis: Binary Heap, Fibonacci Heap, Hollow Heap\n";

        outputFile
            << "  Algorithm: Dijkstra Shortest Path | Dataset: "
            << dataset.label
            << "\n";

        outputFile
            << makeSeparator(100, '=')
            << "\n";

        printExperimentA(outputFile);

        printExperimentB(outputFile);

        outputFile.close();

        std::cout
            << "\n  Results saved to output_results.txt\n";
    }
    else {
        std::cerr
            << "  Failed to write output_results.txt\n";
    }

    std::cout
        << "\n  Finished.\n\n";

    return 0;
}

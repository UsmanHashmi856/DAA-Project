#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <chrono>
#include <string>
#include <iomanip>

#include "src/BinaryHeap.h"
#include "src/FibonacciHeap.h"
#include "src/HollowHeap.h"
#include "src/GraphLoader.h"
#include "src/Dijkstra.h"

static int passedTests = 0;
static int failedTests = 0;

#define check(condition, message)                                \
    do {                                                         \
        if (!(condition)) {                                      \
            std::cout << "  [FAIL] " << message << "\n";         \
            ++failedTests;                                       \
        } else {                                                 \
            std::cout << "  [PASS] " << message << "\n";         \
            ++passedTests;                                       \
        }                                                        \
    } while (0)

template <typename Heap>
std::vector<double> extractAllKeys(Heap& heap) {
    std::vector<double> values;

    while (!heap.empty()) {
        auto [key, id] = heap.extractMin();

        if (id >= 0) {
            values.push_back(key);
        }
    }

    return values;
}

bool isSorted(const std::vector<double>& values) {
    for (int i = 1; i < static_cast<int>(values.size()); ++i) {
        if (values[i] < values[i - 1]) {
            return false;
        }
    }

    return true;
}

template <typename Heap>
void testEmptyHeap(const std::string& heapName) {
    Heap heap;

    check(heap.empty(), heapName + ": empty initially");

    check(
        heap.findMin().second == -1,
        heapName + ": findMin on empty returns id=-1"
    );

    check(
        heap.extractMin().second == -1,
        heapName + ": extractMin on empty returns id=-1"
    );

    check(
        heap.size() == 0,
        heapName + ": size is zero"
    );
}

template <typename Heap>
void testBasicOrder(const std::string& heapName) {
    Heap heap;

    std::vector<int> keys = {
        5, 2, 8, 1, 9,
        3, 7, 4, 6, 0
    };

    for (int key : keys) {
        heap.insert(static_cast<double>(key), key);
    }

    auto result = extractAllKeys(heap);

    check(
        result.size() == 10 && isSorted(result),
        heapName + ": extracted values remain sorted"
    );
}

template <typename Heap>
void testFindMin(const std::string& heapName) {
    Heap heap;

    heap.insert(10.0, 10);

    check(
        heap.findMin().first == 10.0,
        heapName + ": findMin after first insert"
    );

    heap.insert(5.0, 5);

    check(
        heap.findMin().first == 5.0,
        heapName + ": minimum updates correctly"
    );

    heap.insert(7.0, 7);

    check(
        heap.findMin().first == 5.0,
        heapName + ": minimum remains unchanged"
    );
}

template <typename Heap>
void testDecreaseKey(const std::string& heapName) {
    Heap heap;

    for (int i = 0; i < 10; ++i) {
        heap.insert(static_cast<double>(i * 10), i);
    }

    heap.decreaseKey(9, 1.0);

    auto first = heap.extractMin();
    auto second = heap.extractMin();

    check(
        first.second == 0,
        heapName + ": node 0 stays minimum"
    );

    check(
        second.second == 9,
        heapName + ": decreased node moves up"
    );
}

template <typename Heap>
void testDuplicateKeys(const std::string& heapName) {
    Heap heap;

    heap.insert(5.0, 100);
    heap.insert(5.0, 200);
    heap.insert(5.0, 300);
    heap.insert(3.0, 400);

    auto minimum = heap.extractMin();

    check(
        minimum.first == 3.0,
        heapName + ": minimum key extracted correctly"
    );

    auto remaining = extractAllKeys(heap);

    bool valid =
        remaining.size() == 3 &&
        remaining[0] == 5.0 &&
        remaining[1] == 5.0 &&
        remaining[2] == 5.0;

    check(
        valid,
        heapName + ": duplicate keys handled correctly"
    );
}

template <typename Heap>
void testLargeRandom(const std::string& heapName) {
    std::mt19937 rng(42);

    Heap heap;

    std::vector<double> insertedValues;

    for (int i = 0; i < 500; ++i) {
        double key = static_cast<double>(rng() % 100000);

        heap.insert(key, i);
        insertedValues.push_back(key);
    }

    auto extractedValues = extractAllKeys(heap);

    auto sortedValues = insertedValues;

    std::sort(sortedValues.begin(), sortedValues.end());

    check(
        extractedValues == sortedValues,
        heapName + ": random values extracted in order"
    );
}

template <typename Heap>
void testDecreaseKeyReorder(const std::string& heapName) {
    std::mt19937 rng(7);

    Heap heap;

    for (int i = 0; i < 100; ++i) {
        heap.insert(static_cast<double>(rng() % 1000), i);
    }

    for (int i = 0; i < 100; i += 2) {
        heap.decreaseKey(i, -static_cast<double>(i));
    }

    auto result = extractAllKeys(heap);

    check(
        isSorted(result),
        heapName + ": ordering stays valid after decreaseKey"
    );
}

template <typename Heap>
void testDijkstraCorrectness(const std::string& heapName) {
    std::vector<std::vector<Edge>> adjacencyList(4);

    adjacencyList[0] = {
        {1, 1.0},
        {2, 4.0}
    };

    adjacencyList[1] = {
        {0, 1.0},
        {3, 2.0}
    };

    adjacencyList[2] = {
        {0, 4.0},
        {3, 1.0}
    };

    adjacencyList[3] = {
        {1, 2.0},
        {2, 1.0}
    };

    auto result = dijkstra<Heap>(adjacencyList, 0, 4);

    bool valid =
        result.dist[0] == 0.0 &&
        result.dist[1] == 1.0 &&
        result.dist[2] == 4.0 &&
        result.dist[3] == 3.0;

    check(
        valid,
        heapName + ": Dijkstra result is correct"
    );
}

template <typename Heap>
void performanceTest(const std::string& heapName, int operationCount = 50000) {
    using Clock = std::chrono::high_resolution_clock;

    Heap insertHeap;

    auto insertStart = Clock::now();

    for (int i = 0; i < operationCount; ++i) {
        insertHeap.insert(
            static_cast<double>(operationCount - i),
            i
        );
    }

    double insertTime =
        std::chrono::duration<double>(
            Clock::now() - insertStart
        ).count();

    auto extractStart = Clock::now();

    while (!insertHeap.empty()) {
        insertHeap.extractMin();
    }

    double extractTime =
        std::chrono::duration<double>(
            Clock::now() - extractStart
        ).count();

    Heap decreaseHeap;

    for (int i = 0; i < operationCount; ++i) {
        decreaseHeap.insert(
            static_cast<double>(operationCount - i),
            i
        );
    }

    std::mt19937 rng(1);

    std::uniform_int_distribution<int> distribution(
        0,
        operationCount - 1
    );

    int decreaseOperations = 0;

    auto decreaseStart = Clock::now();

    for (int i = 0; i < operationCount / 2; ++i) {
        int id = distribution(rng);

        if (decreaseHeap.contains(id)) {
            decreaseHeap.decreaseKey(
                id,
                -static_cast<double>(id)
            );

            ++decreaseOperations;
        }
    }

    double decreaseTime =
        std::chrono::duration<double>(
            Clock::now() - decreaseStart
        ).count();

    std::cout
        << "  "
        << std::left
        << std::setw(16)
        << heapName
        << std::fixed
        << std::setprecision(3)
        << "  ins="
        << (insertTime / operationCount) * 1e6
        << " us"
        << "  ext="
        << (extractTime / operationCount) * 1e6
        << " us"
        << "  dec="
        << (
            decreaseOperations > 0
                ? (decreaseTime / decreaseOperations) * 1e6
                : 0
        )
        << " us\n";
}

int main() {
    std::cout
        << "\n"
        << std::string(62, '=')
        << "\n";

    std::cout
        << "  CORRECTNESS TESTS\n";

    std::cout
        << std::string(62, '=')
        << "\n\n";

#define runHeapTests(heapType, label)                  \
    std::cout << "  -- " << label << " --\n";          \
    testEmptyHeap<heapType>(label);                    \
    testBasicOrder<heapType>(label);                   \
    testFindMin<heapType>(label);                      \
    testDecreaseKey<heapType>(label);                  \
    testDuplicateKeys<heapType>(label);                \
    testLargeRandom<heapType>(label);                  \
    testDecreaseKeyReorder<heapType>(label);           \
    testDijkstraCorrectness<heapType>(label);          \
    std::cout << "\n";

    runHeapTests(BinaryHeap, "BinaryHeap")
    runHeapTests(FibonacciHeap, "FibonacciHeap")
    runHeapTests(HollowHeap, "HollowHeap")

#undef runHeapTests

    std::cout
        << std::string(62, '=')
        << "\n";

    std::cout
        << "  Results: "
        << passedTests
        << " passed, "
        << failedTests
        << " failed\n";

    std::cout
        << std::string(62, '=')
        << "\n\n";

    if (passedTests + failedTests > 0 && failedTests == 0) {
        std::cout
            << "  All correctness tests passed.\n\n";
    }

    std::cout
        << std::string(62, '=')
        << "\n";

    std::cout
        << "  PERFORMANCE TESTS (50,000 operations)\n";

    std::cout
        << std::string(62, '=')
        << "\n\n";

    performanceTest<BinaryHeap>("BinaryHeap");
    performanceTest<FibonacciHeap>("FibonacciHeap");
    performanceTest<HollowHeap>("HollowHeap");

    std::cout << "\n";

    return failedTests == 0 ? 0 : 1;
}

#pragma once

#include <vector>
#include <limits>
#include <chrono>

struct DijkstraResult {
    std::vector<double> dist;

    double insertTime = 0;
    double extractTime = 0;
    double decreaseTime = 0;
    double totalTime = 0;

    long long insertCount = 0;
    long long extractCount = 0;
    long long decreaseCount = 0;

    double avgInsertUs = 0;
    double avgExtractUs = 0;
    double avgDecreaseUs = 0;

    int peakHeight = 0;
    int peakTrees = 0;
    int peakSize = 0;
};

template <typename Heap>
DijkstraResult dijkstra(
    const std::vector<std::vector<Edge>>& adjacencyList,
    int source,
    int nodeCount
) {
    using Clock = std::chrono::high_resolution_clock;

    constexpr double infinity =
        std::numeric_limits<double>::infinity();

    DijkstraResult result;

    result.dist.assign(nodeCount, infinity);
    result.dist[source] = 0.0;

    Heap heap;

    heap.insert(0.0, source);

    std::vector<bool> activeInHeap(nodeCount, false);

    activeInHeap[source] = true;

    auto startTime = Clock::now();

    while (!heap.empty()) {
        if (heap.size() > result.peakSize) {
            result.peakSize = heap.size();
            result.peakHeight = heap.height();
            result.peakTrees = heap.numTrees();
        }

        auto extractStart = Clock::now();

        auto [currentDistance, currentNode] =
            heap.extractMin();

        result.extractTime +=
            std::chrono::duration<double>(
                Clock::now() - extractStart
            ).count();

        ++result.extractCount;

        if (currentNode < 0) {
            break;
        }

        activeInHeap[currentNode] = false;

        if (currentDistance > result.dist[currentNode]) {
            continue;
        }

        for (const Edge& edge : adjacencyList[currentNode]) {
            int nextNode = edge.to;

            double newDistance =
                result.dist[currentNode] + edge.weight;

            if (newDistance < result.dist[nextNode]) {
                result.dist[nextNode] = newDistance;

                if (!activeInHeap[nextNode]) {
                    auto insertStart = Clock::now();

                    heap.insert(newDistance, nextNode);

                    result.insertTime +=
                        std::chrono::duration<double>(
                            Clock::now() - insertStart
                        ).count();

                    ++result.insertCount;

                    activeInHeap[nextNode] = true;
                } else {
                    auto decreaseStart = Clock::now();

                    heap.decreaseKey(nextNode, newDistance);

                    result.decreaseTime +=
                        std::chrono::duration<double>(
                            Clock::now() - decreaseStart
                        ).count();

                    ++result.decreaseCount;
                }
            }
        }
    }

    result.totalTime =
        std::chrono::duration<double>(
            Clock::now() - startTime
        ).count();

    if (result.insertCount > 0) {
        result.avgInsertUs =
            (result.insertTime / result.insertCount) * 1e6;
    }

    if (result.extractCount > 0) {
        result.avgExtractUs =
            (result.extractTime / result.extractCount) * 1e6;
    }

    if (result.decreaseCount > 0) {
        result.avgDecreaseUs =
            (result.decreaseTime / result.decreaseCount) * 1e6;
    }

    return result;
}

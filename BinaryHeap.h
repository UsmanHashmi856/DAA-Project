#pragma once

#include <vector>
#include <unordered_map>
#include <stdexcept>
#include <limits>

class BinaryHeap {
public:
    long long insertCount = 0;
    long long extractCount = 0;
    long long decreaseCount = 0;

    BinaryHeap() = default;

    void insert(double key, int nodeId) {
        ++insertCount;

        int index = static_cast<int>(heapData.size());

        heapData.push_back({key, nodeId});
        positions[nodeId] = index;

        siftUp(index);
    }

    std::pair<double, int> findMin() const {
        if (heapData.empty()) {
            return {
                std::numeric_limits<double>::infinity(),
                -1
            };
        }

        return {
            heapData[0].key,
            heapData[0].id
        };
    }

    std::pair<double, int> extractMin() {
        if (heapData.empty()) {
            return {
                std::numeric_limits<double>::infinity(),
                -1
            };
        }

        ++extractCount;

        std::pair<double, int> minimum = {
            heapData[0].key,
            heapData[0].id
        };

        int lastIndex =
            static_cast<int>(heapData.size()) - 1;

        swapNodes(0, lastIndex);

        heapData.pop_back();

        positions.erase(minimum.second);

        if (!heapData.empty()) {
            siftDown(0);
        }

        return minimum;
    }

    void decreaseKey(int nodeId, double newKey) {
        ++decreaseCount;

        auto iterator = positions.find(nodeId);

        if (iterator == positions.end()) {
            return;
        }

        int index = iterator->second;

        if (newKey >= heapData[index].key) {
            return;
        }

        heapData[index].key = newKey;

        siftUp(index);
    }

    void deleteNode(int nodeId) {
        decreaseKey(
            nodeId,
            -std::numeric_limits<double>::infinity()
        );

        extractMin();
    }

    bool contains(int nodeId) const {
        return positions.count(nodeId) > 0;
    }

    int size() const {
        return static_cast<int>(heapData.size());
    }

    bool empty() const {
        return heapData.empty();
    }

    int numTrees() const {
        return heapData.empty() ? 0 : 1;
    }

    int height() const {
        if (heapData.empty()) {
            return 0;
        }

        int nodeCount =
            static_cast<int>(heapData.size());

        int currentHeight = 0;
        int nodesPerLevel = 1;

        while (nodesPerLevel < nodeCount) {
            nodesPerLevel *= 2;
            ++currentHeight;
        }

        return currentHeight;
    }

    void resetCounters() {
        insertCount = 0;
        extractCount = 0;
        decreaseCount = 0;
    }

private:
    struct HeapEntry {
        double key;
        int id;
    };

    std::vector<HeapEntry> heapData;

    std::unordered_map<int, int> positions;

    void swapNodes(int firstIndex, int secondIndex) {
        positions[heapData[firstIndex].id] = secondIndex;
        positions[heapData[secondIndex].id] = firstIndex;

        std::swap(
            heapData[firstIndex],
            heapData[secondIndex]
        );
    }

    void siftUp(int index) {
        while (index > 0) {
            int parentIndex = (index - 1) / 2;

            if (
                heapData[parentIndex].key >
                heapData[index].key
            ) {
                swapNodes(index, parentIndex);

                index = parentIndex;
            } else {
                break;
            }
        }
    }

    void siftDown(int index) {
        int heapSize =
            static_cast<int>(heapData.size());

        while (true) {
            int smallestIndex = index;

            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;

            if (
                leftChild < heapSize &&
                heapData[leftChild].key <
                heapData[smallestIndex].key
            ) {
                smallestIndex = leftChild;
            }

            if (
                rightChild < heapSize &&
                heapData[rightChild].key <
                heapData[smallestIndex].key
            ) {
                smallestIndex = rightChild;
            }

            if (smallestIndex != index) {
                swapNodes(index, smallestIndex);

                index = smallestIndex;
            } else {
                break;
            }
        }
    }
};

#pragma once

#include <vector>
#include <unordered_map>
#include <limits>
#include <stack>

static constexpr int hollowNodeId = -1;

class HollowHeap {
public:
    long long insertCount = 0;
    long long extractCount = 0;
    long long decreaseCount = 0;

    HollowHeap() = default;

    ~HollowHeap() {
        for (Node* node : allocatedNodes) {
            delete node;
        }
    }

    void insert(double key, int nodeId) {
        ++insertCount;

        Node* node = createNode(key, nodeId);

        activeNodes[nodeId] = node;

        roots.push_back(node);

        ++nodeCount;
    }

    std::pair<double, int> findMin() {
        if (nodeCount == 0) {
            return {
                std::numeric_limits<double>::infinity(),
                -1
            };
        }

        minimumNode = nullptr;

        refreshMinimum();

        if (!minimumNode) {
            return {
                std::numeric_limits<double>::infinity(),
                -1
            };
        }

        return {
            minimumNode->key,
            minimumNode->id
        };
    }

    std::pair<double, int> extractMin() {
        if (nodeCount == 0) {
            return {
                std::numeric_limits<double>::infinity(),
                -1
            };
        }

        ++extractCount;

        refreshMinimum();

        double minimumKey = minimumNode->key;
        int minimumId = minimumNode->id;

        activeNodes.erase(minimumId);

        minimumNode->id = hollowNodeId;

        --nodeCount;

        consolidate();

        minimumNode = nullptr;

        return {
            minimumKey,
            minimumId
        };
    }

    void decreaseKey(int nodeId, double newKey) {
        ++decreaseCount;

        Node* previousNode = activeNodes[nodeId];

        if (newKey >= previousNode->key) {
            return;
        }

        Node* replacementNode =
            createNode(newKey, nodeId);

        replacementNode->rank =
            previousNode->rank > 2
                ? previousNode->rank - 2
                : 0;

        activeNodes[nodeId] = replacementNode;

        previousNode->id = hollowNodeId;

        roots.push_back(replacementNode);
    }

    void deleteNode(int nodeId) {
        decreaseKey(
            nodeId,
            -std::numeric_limits<double>::infinity()
        );

        extractMin();
    }

    bool contains(int nodeId) const {
        return activeNodes.count(nodeId) > 0;
    }

    int size() const {
        return nodeCount;
    }

    bool empty() const {
        return nodeCount == 0;
    }

    int numTrees() const {
        int liveRoots = 0;

        for (Node* root : roots) {
            if (root->id != hollowNodeId) {
                ++liveRoots;
            }
        }

        if (liveRoots == 0 && nodeCount > 0) {
            return 1;
        }

        return liveRoots;
    }

    int height() const {
        int maximumHeight = 0;

        for (Node* root : roots) {
            if (root->id != hollowNodeId) {
                int currentHeight =
                    calculateSubtreeHeight(root);

                if (currentHeight > maximumHeight) {
                    maximumHeight = currentHeight;
                }
            }
        }

        return maximumHeight;
    }

    void resetCounters() {
        insertCount = 0;
        extractCount = 0;
        decreaseCount = 0;
    }

private:
    struct Node {
        double key;
        int id;
        int rank = 0;

        Node* child = nullptr;
        Node* right = nullptr;
    };

    std::vector<Node*> roots;

    std::unordered_map<int, Node*> activeNodes;

    std::vector<Node*> allocatedNodes;

    Node* minimumNode = nullptr;

    int nodeCount = 0;

    Node* createNode(double key, int id) {
        Node* node = new Node();

        node->key = key;
        node->id = id;

        allocatedNodes.push_back(node);

        return node;
    }

    void refreshMinimum() {
        if (minimumNode != nullptr) {
            return;
        }

        for (Node* root : roots) {
            if (root->id != hollowNodeId) {
                if (
                    minimumNode == nullptr ||
                    root->key < minimumNode->key
                ) {
                    minimumNode = root;
                }
            }
        }
    }

    static Node* link(Node* first, Node* second) {
        if (first->key <= second->key) {
            second->right = first->child;

            first->child = second;

            ++first->rank;

            return first;
        }

        first->right = second->child;

        second->child = first;

        ++second->rank;

        return second;
    }

    void consolidate() {
        std::vector<Node*> rankTable(64, nullptr);

        auto ensureCapacity = [&](int rank) {
            while (
                rank >= static_cast<int>(rankTable.size())
            ) {
                rankTable.push_back(nullptr);
            }
        };

        std::stack<Node*> pending;

        for (Node* root : roots) {
            pending.push(root);
        }

        roots.clear();

        while (!pending.empty()) {
            Node* current = pending.top();

            pending.pop();

            if (current->id == hollowNodeId) {
                Node* child = current->child;

                while (child) {
                    Node* nextChild = child->right;

                    child->right = nullptr;

                    pending.push(child);

                    child = nextChild;
                }
            } else {
                current->right = nullptr;

                int rank = current->rank;

                current->rank = calculateRank(current);

                ensureCapacity(rank);

                while (rankTable[rank] != nullptr) {
                    Node* other = rankTable[rank];

                    rankTable[rank] = nullptr;

                    current = link(current, other);

                    ++rank;

                    ensureCapacity(rank);
                }

                rankTable[rank] = current;
            }
        }

        for (Node* node : rankTable) {
            if (node) {
                node->right = nullptr;

                roots.push_back(node);
            }
        }
    }

    int calculateRank(Node* node) const {
        int rank = 0;

        Node* child = node->child;

        while (child) {
            ++rank;

            child = child->right;
        }

        return rank;
    }

    int calculateSubtreeHeight(
        Node* node,
        int depth = 0
    ) const {
        if (!node || depth > 80) {
            return depth;
        }

        int maximumHeight = depth;

        Node* child = node->child;

        int limit = 0;

        while (child && limit < 10) {
            int currentHeight =
                calculateSubtreeHeight(
                    child,
                    depth + 1
                );

            if (currentHeight > maximumHeight) {
                maximumHeight = currentHeight;
            }

            child = child->right;

            ++limit;
        }

        return maximumHeight;
    }
};

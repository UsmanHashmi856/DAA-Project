#pragma once

#include <vector>
#include <unordered_map>
#include <cmath>
#include <limits>

class FibonacciHeap {
public:
    long long insertCount = 0;
    long long extractCount = 0;
    long long decreaseCount = 0;
    long long cascadingCuts = 0;

    FibonacciHeap() = default;

    ~FibonacciHeap() {
        for (auto& entry : nodeLookup) {
            delete entry.second;
        }

        nodeLookup.clear();
    }

    void insert(double key, int nodeId) {
        ++insertCount;

        Node* node = new Node(key, nodeId);

        nodeLookup[nodeId] = node;

        addToRootList(node);

        ++rootCount;
        ++nodeCount;

        if (minimumNode == nullptr || node->key < minimumNode->key) {
            minimumNode = node;
        }
    }

    std::pair<double, int> findMin() const {
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
        if (!minimumNode) {
            return {
                std::numeric_limits<double>::infinity(),
                -1
            };
        }

        ++extractCount;

        Node* child = minimumNode->child;

        if (child) {
            std::vector<Node*> children;

            Node* current = child;

            do {
                children.push_back(current);
                current = current->right;
            } while (current != child);

            for (Node* currentChild : children) {
                removeFromList(currentChild);

                addToRootList(currentChild);

                currentChild->parent = nullptr;
                currentChild->marked = false;

                ++rootCount;
            }

            minimumNode->child = nullptr;
        }

        std::pair<double, int> result = {
            minimumNode->key,
            minimumNode->id
        };

        nodeLookup.erase(minimumNode->id);

        --rootCount;
        --nodeCount;

        if (minimumNode->right == minimumNode) {
            delete minimumNode;

            minimumNode = nullptr;
        } else {
            Node* nextNode = minimumNode->right;

            removeFromList(minimumNode);

            delete minimumNode;

            minimumNode = nextNode;

            consolidate();
        }

        return result;
    }

    void decreaseKey(int nodeId, double newKey) {
        ++decreaseCount;

        Node* node = nodeLookup[nodeId];

        if (newKey >= node->key) {
            return;
        }

        node->key = newKey;

        Node* parent = node->parent;

        if (parent && node->key < parent->key) {
            cut(node, parent);

            cascadingCut(parent);
        }

        if (node->key < minimumNode->key) {
            minimumNode = node;
        }
    }

    void deleteNode(int nodeId) {
        decreaseKey(
            nodeId,
            -std::numeric_limits<double>::infinity()
        );

        extractMin();
    }

    bool contains(int nodeId) const {
        return nodeLookup.count(nodeId) > 0;
    }

    int size() const {
        return nodeCount;
    }

    bool empty() const {
        return nodeCount == 0;
    }

    int numTrees() const {
        return rootCount;
    }

    int height() const {
        if (!minimumNode) {
            return 0;
        }

        int maximumHeight = 0;

        Node* current = minimumNode;

        do {
            int currentHeight = calculateNodeHeight(current);

            if (currentHeight > maximumHeight) {
                maximumHeight = currentHeight;
            }

            current = current->right;
        } while (current != minimumNode);

        return maximumHeight;
    }

    void resetCounters() {
        insertCount = 0;
        extractCount = 0;
        decreaseCount = 0;
        cascadingCuts = 0;
    }

private:
    struct Node {
        double key;
        int id;
        int degree = 0;

        bool marked = false;

        Node* parent = nullptr;
        Node* child = nullptr;
        Node* left = nullptr;
        Node* right = nullptr;

        Node(double nodeKey, int nodeId)
            : key(nodeKey),
              id(nodeId),
              left(this),
              right(this) {}
    };

    Node* minimumNode = nullptr;

    int nodeCount = 0;
    int rootCount = 0;

    std::unordered_map<int, Node*> nodeLookup;

    void addToRootList(Node* node) {
        if (minimumNode == nullptr) {
            node->left = node;
            node->right = node;

            minimumNode = node;
        } else {
            node->right = minimumNode;
            node->left = minimumNode->left;

            minimumNode->left->right = node;
            minimumNode->left = node;
        }

        node->parent = nullptr;
    }

    void removeFromList(Node* node) {
        node->left->right = node->right;
        node->right->left = node->left;

        node->left = node;
        node->right = node;
    }

    void link(Node* child, Node* parent) {
        removeFromList(child);

        child->parent = parent;
        child->marked = false;

        if (parent->child == nullptr) {
            parent->child = child;

            child->left = child;
            child->right = child;
        } else {
            child->right = parent->child;
            child->left = parent->child->left;

            parent->child->left->right = child;
            parent->child->left = child;
        }

        ++parent->degree;
        --rootCount;
    }

    void consolidate() {
        int maxDegree =
            static_cast<int>(
                std::log2(nodeCount + 1)
            ) + 2;

        std::vector<Node*> degreeTable(
            maxDegree + 1,
            nullptr
        );

        std::vector<Node*> roots;

        Node* current = minimumNode;

        do {
            roots.push_back(current);

            current = current->right;
        } while (current != minimumNode);

        for (Node* root : roots) {
            Node* currentNode = root;

            int degree = currentNode->degree;

            while (
                degree < static_cast<int>(degreeTable.size()) &&
                degreeTable[degree] != nullptr
            ) {
                Node* other = degreeTable[degree];

                if (currentNode->key > other->key) {
                    std::swap(currentNode, other);
                }

                link(other, currentNode);

                degreeTable[degree] = nullptr;

                ++degree;

                if (degree >= static_cast<int>(degreeTable.size())) {
                    degreeTable.push_back(nullptr);
                }
            }

            if (degree >= static_cast<int>(degreeTable.size())) {
                degreeTable.resize(degree + 1, nullptr);
            }

            degreeTable[degree] = currentNode;
        }

        minimumNode = nullptr;
        rootCount = 0;

        for (Node* node : degreeTable) {
            if (!node) {
                continue;
            }

            node->left = node;
            node->right = node;

            ++rootCount;

            if (minimumNode == nullptr) {
                minimumNode = node;
            } else {
                node->right = minimumNode;
                node->left = minimumNode->left;

                minimumNode->left->right = node;
                minimumNode->left = node;

                if (node->key < minimumNode->key) {
                    minimumNode = node;
                }
            }
        }
    }

    void cut(Node* node, Node* parent) {
        if (node->right == node) {
            parent->child = nullptr;
        } else {
            if (parent->child == node) {
                parent->child = node->right;
            }

            removeFromList(node);
        }

        --parent->degree;

        addToRootList(node);

        ++rootCount;

        node->marked = false;
    }

    void cascadingCut(Node* node) {
        Node* parent = node->parent;

        if (parent) {
            if (!node->marked) {
                node->marked = true;
            } else {
                ++cascadingCuts;

                cut(node, parent);

                cascadingCut(parent);
            }
        }
    }

    int calculateNodeHeight(Node* node) const {
        if (!node->child) {
            return 0;
        }

        int maximumHeight = 0;

        Node* current = node->child;

        do {
            int currentHeight =
                calculateNodeHeight(current) + 1;

            if (currentHeight > maximumHeight) {
                maximumHeight = currentHeight;
            }

            current = current->right;
        } while (current != node->child);

        return maximumHeight;
    }
};

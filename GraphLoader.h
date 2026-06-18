#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <algorithm>

struct Edge {
    int to;
    double weight;
};

struct Graph {
    int numNodes = 0;
    int numEdges = 0;

    std::vector<std::vector<Edge>> adj;
};

inline Graph loadGraph(const std::string& filePath) {
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error(
            "Cannot open file: " + filePath
        );
    }

    Graph graph;

    std::string line;

    std::vector<std::tuple<int, int, double>> edges;

    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        if (line[0] == '#') {
            auto vertexPosition = line.find("vertices");
            auto edgePosition = line.find("edges");

            if (
                vertexPosition != std::string::npos &&
                edgePosition != std::string::npos
            ) {
                std::istringstream stream(line);

                std::string token;

                std::vector<long long> values;

                while (stream >> token) {
                    try {
                        long long number =
                            std::stoll(token);

                        values.push_back(number);
                    } catch (...) {
                    }
                }

                if (values.size() >= 2) {
                    graph.numNodes =
                        static_cast<int>(values[0]);

                    graph.numEdges =
                        static_cast<int>(values[1]);
                }
            }

            continue;
        }

        std::istringstream stream(line);

        int from;
        int to;

        double weight;

        if (!(stream >> from >> to >> weight)) {
            continue;
        }

        edges.emplace_back(from, to, weight);
    }

    if (graph.numNodes == 0 && !edges.empty()) {
        int maxNodeId = 0;

        for (const auto& [from, to, weight] : edges) {
            maxNodeId =
                std::max({maxNodeId, from, to});
        }

        graph.numNodes = maxNodeId + 1;
    }

    graph.adj.assign(graph.numNodes, {});

    for (const auto& [from, to, weight] : edges) {
        graph.adj[from].push_back({to, weight});

        graph.adj[to].push_back({from, weight});
    }

    if (graph.numEdges == 0) {
        graph.numEdges =
            static_cast<int>(edges.size());
    }

    return graph;
}

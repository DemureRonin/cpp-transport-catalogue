#pragma once
#include <vector>
#include <string>
#include <optional>
#include <stdexcept>

namespace graph {
    using VertexId = size_t;
    using EdgeId = size_t;

    template<typename Weight>
    struct Edge {
        VertexId from;
        VertexId to;
        Weight weight;


        std::string bus_name;
        int span_count = 0;
    };

    template<typename Weight>
    class DirectedWeightedGraph {
    public:
        VertexId AddVertex() {
            adjacency_list_.emplace_back();
            return adjacency_list_.size() - 1;
        }

        void Resize(size_t vertex_count) {
            adjacency_list_.resize(vertex_count);
        }

        EdgeId AddEdge(const Edge<Weight> &edge) {
            edges_.push_back(edge);
            adjacency_list_[edge.from].push_back(edges_.size() - 1);
            return edges_.size() - 1;
        }

        const Edge<Weight> &GetEdge(EdgeId id) const {
            return edges_.at(id);
        }

        const std::vector<EdgeId> &GetIncidentEdges(VertexId v) const {
            return adjacency_list_.at(v);
        }

        size_t GetVertexCount() const {
            return adjacency_list_.size();
        }

    private:
        std::vector<Edge<Weight> > edges_;
        std::vector<std::vector<EdgeId> > adjacency_list_;
    };
}

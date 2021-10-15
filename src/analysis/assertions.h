#pragma once

#include <geometry/line_graph.h>

namespace omg {
namespace analysis {

bool isClosed(const LineGraph& lg);

bool hasValenceGreaterTwo(const LineGraph& lg);

bool hasUnusedVertices(const LineGraph& lg);

bool hasOneVertexLoops(const LineGraph& lg);

std::size_t countTwoVertexLoops(const LineGraph& lg);

std::size_t countZeroLengthEdges(const LineGraph& lg);

std::size_t countIdenticalPoints(const LineGraph& lg);

std::size_t countCollinearEdges(const LineGraph& lg);

std::size_t countAngles(const LineGraph& lg, real_t degrees);

}
}

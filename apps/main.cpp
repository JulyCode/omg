#include <iostream>

#include <geometry/polygon.h>
#include <io/poly_reader.h>

int main()
{
    std::cout << "Hello World!" << std::endl;

    omg::Polygon poly = omg::io::readPoly("../../apps/medsea.poly");
    std::cout << "input polygon:" << std::endl;
    std::cout << "vertices: " << poly.getVertices().size() << std::endl;
    std::cout << "edges: " << poly.getEdges().size() << std::endl;
}
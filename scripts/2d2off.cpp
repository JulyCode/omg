
// Author: Daniel Zint

#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string>

#include <vector>
#include <algorithm>
#include <functional>
#include <cctype>
#include <set>

void deleteSpaces(std::string &str)
{
    // delete spaces at the beginning of string
	str.erase(str.begin(), std::find_if(str.begin(), str.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));
	// delte spaces at the end of string
	str.erase(std::find_if(str.rbegin(), str.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
}

bool getLine(std::ifstream& ifs, std::string& line)
{
	if (!std::getline(ifs, line))
		return false;

    deleteSpaces(line);
	while (line.size() == 0)
	{
		if (!std::getline(ifs, line))
		{
			return false;
		}
        deleteSpaces(line);
	}
	return true;
}

void errorMsg(const std::string & msg, const std::string& file, const std::string& function, const int& line)
{
	std::cerr << "ERROR: " << msg << std::endl;
	std::cerr << "ERROR: " << std::endl;
	std::cerr << "\t file:     " << file << std::endl;
	std::cerr << "\t function: " << function << std::endl;
	std::cerr << "\t line:     " << line << std::endl;
}

#define errorMsg(msg) errorMsg(msg, __FILE__, __FUNCTION__, __LINE__)

void read2dNod(const std::string& filename, std::vector<std::vector<float>>& vertex){
	std::ifstream ifs(filename);

    if (!ifs.is_open()) {
		std::ostringstream ss;
		ss << "Could not load file. Check file-path!" << std::endl;
		ss << "\t filename = \"" << filename << "\"";
		errorMsg(ss.str());
		exit(-1);
	}
	
	std::string line;
	    
    int nVertices;
    getLine(ifs, line);     // nVertices
	std::istringstream iss(line);
    iss >> nVertices;
	
	for(int i = 0; i < nVertices; ++i){
        getLine(ifs, line);
        std::istringstream iss(line);
		float buf;
        float x,y,z;
        iss >> buf >> x >> y >> z;
        vertex.push_back({x,y,z});
    }
}

void read2dElem(const std::string& filename, std::vector<std::vector<int>>& element){
	std::ifstream ifs(filename);

    if (!ifs.is_open()) {
		std::ostringstream ss;
		ss << "Could not load file. Check file-path!" << std::endl;
		ss << "\t filename = \"" << filename << "\"";
		errorMsg(ss.str());
		exit(-1);
	}
	
	std::string line;
	    
    int nElems;
    getLine(ifs, line);     // nElems
	std::istringstream iss(line);
    iss >> nElems;
	
	for(int i = 0; i < nElems; ++i){
        getLine(ifs, line);
        std::istringstream iss(line);
        int x,y,z;
        iss >> x >> y >> z;
        element.push_back({x-1,y-1,z-1});
    }
}

int main(int argc, char **argv){
    //if(argc != 2){
    //    std::cout << "Invalid number of arguments" << std::endl;
    //}
    //const std::string filename = argv[1];

    std::vector<std::vector<float>> vertex;
    std::vector<std::vector<int>> element;

    const std::string elemFilename = "elem2d.out";
	const std::string nodFilename = "nod2d.out";
    
	read2dNod(nodFilename, vertex);
	read2dElem(elemFilename, element);
	
    std::string outName = "output.off";
    
    std::ofstream ofs(outName);
	ofs << "OFF" << std::endl;
	ofs << vertex.size() << " " << element.size() << " " << 0 << std::endl;
	for(auto v : vertex){
		//ofs << v[0] << " " << v[1] << " " << v[2] << std::endl;
		ofs << v[0] << " " << v[1] << " " << 0 << std::endl;
	}
	for(auto e : element){
		ofs << 3 << " " << e[0] << " " << e[1] << " " << e[2] << std::endl;
	}
	
	ofs.close();
	
	
    //ofs << element.size() << "\r\n";
    //for(auto q : qualitySet){
    //    ofs << q << "\r\n";
    //}
    //ofs.close();

}











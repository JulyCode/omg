
#include "vtk_writer.h"

#include <fstream>

namespace omg {
namespace io {

template<typename T>
static void swapEndianess(T& var) {
	char* varArray = reinterpret_cast<char*>(&var);

	for (std::size_t i = 0; i < sizeof(var) / 2; i++) {
        std::swap(varArray[sizeof(var) - 1 - i], varArray[i]);
    }
}

template<typename T>
static void writeLegacyVTK(const std::string& filename, const ScalarField<T>& data,
                           bool binary, const std::string& type) {

    const size2_t& grid_size = data.getGridSize();
	const vec2_t& cell_size = data.getCellSize();
    const AxisAlignedBoundingBox& aabb = data.getBoundingBox();

	std::ofstream file;
	if (binary) {
	    file.open(filename, std::ios::binary | std::ios::out);
    } else {
		file.open(filename);
    }

	file << "# vtk DataFile Version 3.0" << std::endl;
	file << filename << std::endl;

	if (binary) {
		file << "BINARY" << std::endl;
	} else {
		file << "ASCII" << std::endl;
    }

	file << "DATASET STRUCTURED_POINTS" << std::endl;
	file << "DIMENSIONS " << grid_size[0] << " " << grid_size[1] << " " << 1 << std::endl;
	file << "ORIGIN " << aabb.min[0] << " " << aabb.min[1] << " " << 0 << std::endl;
	file << "SPACING " << cell_size[0] << " " << cell_size[1] << " " << 1 << std::endl;

	file << "POINT_DATA " << data.grid().size() << std::endl;
	file << "SCALARS value " << type << std::endl;
	file << "LOOKUP_TABLE data_table" << std::endl;

	for (std::size_t j = 0; j < grid_size[1]; j++) {
		for (std::size_t i = 0; i < grid_size[0]; i++) {

			if (binary) {
				T val = data.grid(i, j);
				swapEndianess(val);

				file.write(reinterpret_cast<char*>(&val), sizeof(val));
			} else {
				file << data.grid(i, j) << "\n";
			}
		}
	}
	if (binary) {
		file << "\n";
    }

	file.close();
}

template<>
void writeLegacyVTK<float>(const std::string& filename, const ScalarField<float>& data, bool binary) {
    writeLegacyVTK(filename, data, binary, "float 1");
}

template<>
void writeLegacyVTK<double>(const std::string& filename, const ScalarField<double>& data, bool binary) {
    writeLegacyVTK(filename, data, binary, "double 1");
}

template<>
void writeLegacyVTK<int16_t>(const std::string& filename, const ScalarField<int16_t>& data, bool binary) {
    writeLegacyVTK(filename, data, binary, "short 1");
}

}
}

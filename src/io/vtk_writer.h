#pragma once

#include <topology/scalar_field.h>

namespace omg {
namespace io {

template<typename T>
void writeLegacyVTK(const std::string& filename, const ScalarField<T>& data, bool binary);

}
}

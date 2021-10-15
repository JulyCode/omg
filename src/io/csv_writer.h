#pragma once

#include <string>
#include <vector>
#include <fstream>

namespace omg {
namespace io {

class CSVTable {
public:
    explicit CSVTable(const std::string delim = ";") : delim(delim) {}

    const std::string& get(std::size_t row, std::size_t col) const {
        if (col >= columns.size()) {
            throw std::runtime_error("invalid column index");
        }
        if (row >= data.size()) {
            throw std::runtime_error("invalid row index");
        }

        return data[row][col];
    }

    template<typename T>
    void set(std::size_t row, std::size_t col, T value) {
        if (col >= columns.size()) {
            throw std::runtime_error("invalid column index");
        }
        if (row >= data.size()) {
            throw std::runtime_error("invalid row index");
        }

        data[row][col] = std::to_string(value);
    }


    std::size_t addRow() {
        data.push_back(std::vector<std::string>(columns.size()));
        return data.size() - 1;
    }

    template<typename Iter>
    void addRow(Iter begin, Iter end) {

        const std::size_t size = std::distance(begin, end);

        if (size > columns.size()) {
            throw std::runtime_error("table has not enough columns");
        }

        // insert new row
        addRow();
        auto& row = data.back();

        for (std::size_t i = 0; begin != end; ++begin, i++) {

            row[i] = std::to_string(*begin);
        }
    }

    void addColumn(const std::string& name) {
        columns.push_back(name);

        // fill existing rows
        for (auto& row : data) {
            row.push_back("");
        }
    }

    template<typename Iter>
    void addColumn(const std::string& name, Iter begin, Iter end) {
        columns.push_back(name);

        const std::size_t size = std::distance(begin, end);
        const std::size_t prev_size = data.size();

        if (size > prev_size) {
            data.resize(size);  // add more rows if necessary
        }

        for (std::size_t i = 0; begin != end; ++begin, i++) {

            auto& row = data[i];

            if (i < prev_size) {
                // append to existing row
                row.push_back(std::to_string(*begin));
            } else {
                // fill new row
                row.resize(columns.size());
                row.back() = std::to_string(*begin);
            }
        }

        // fill remaining rows if necessary
        for (std::size_t i = size; i < prev_size; i++) {
            data[i].push_back("");
        }
    }


    void write(const std::string& filename) const {

        std::ofstream file(filename);

        if (!file.good()) {
            throw std::runtime_error("Error writing to file: " + filename);
        }

        writeRow(file, columns);

        for (const auto& row : data) {
            writeRow(file, row);
        }

        file.close();
    }

    // TODO: void read(const std::string& filename);

private:
    const std::string delim;

    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> data;  // row-major

    void writeRow(std::ofstream& file, const std::vector<std::string>& row) const {
        for (std::size_t i = 0; i < row.size(); i++) {
            file << row[i];

            if (i != row.size() - 1) {
                file << delim;
            }
        }
        file << "\n";
    }
};


template<typename T>
void writeCSV(const std::string& filename, const std::vector<T>& data, const std::string& col_name = "") {

    std::ofstream file(filename);

    if (!file.good()) {
        throw std::runtime_error("Error writing to file: " + filename);
    }

    if (!col_name.empty()) {
        file << col_name << std::endl;
    }

    for (const T& d : data) {
        file << std::to_string(d) << std::endl;
    }

    file.close();
}

}
}

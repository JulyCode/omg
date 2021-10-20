
#include "gradient_limiting.h"

#include <iostream>
#include <list>

#include <util.h>
#include <size_function/jigsaw_size.h>

#include <jigsaw/inc/lib_jigsaw.h>

namespace omg {

static vec2_t forwardDifference(const ScalarField<real_t>& size, size2_t idx) {
    vec2_t diff(0);

    if (idx[0] + 1 < size.getGridSize()[0]) {
        diff[0] = size.grid(idx[0] + 1, idx[1]) - size.grid(idx);
    }

    if (idx[1] + 1 < size.getGridSize()[1]) {
        diff[1] = size.grid(idx[0], idx[1] + 1) - size.grid(idx);
    }

    return diff / size.getCellSize();
}

static vec2_t backwardDifference(const ScalarField<real_t>& size, size2_t idx) {
    vec2_t diff(0);

    if (idx[0] > 0) {
        diff[0] = size.grid(idx) - size.grid(idx[0] - 1, idx[1]);
    }

    if (idx[1] > 0) {
        diff[1] = size.grid(idx) - size.grid(idx[0], idx[1] - 1);
    }

    return diff / size.getCellSize();
}

static void allDifferences(const ScalarField<real_t>& size, size2_t idx, std::vector<real_t>& grad) {
    std::fill(grad.begin(), grad.end(), 0);

    const real_t dia = size.getCellSize().norm();

    if (idx[0] > 0) {
        grad[0] = (size.grid(idx) - size.grid(idx[0] - 1, idx[1])) / size.getCellSize()[0];

        if (idx[1] > 0) {
            grad[4] = (size.grid(idx) - size.grid(idx[0] - 1, idx[1] - 1)) / dia;
        }

        if (idx[1] + 1 < size.getGridSize()[1]) {
            grad[5] = (size.grid(idx[0] - 1, idx[1] + 1) - size.grid(idx)) / dia;
        }
    }

    if (idx[1] > 0) {
        grad[1] = (size.grid(idx) - size.grid(idx[0], idx[1] - 1)) / size.getCellSize()[0];
    }

    if (idx[0] + 1 < size.getGridSize()[0]) {
        grad[2] = (size.grid(idx[0] + 1, idx[1]) - size.grid(idx)) / size.getCellSize()[1];

        if (idx[1] > 0) {
            grad[6] = (size.grid(idx[0] + 1, idx[1] - 1) - size.grid(idx)) / dia;
        }

        if (idx[1] + 1 < size.getGridSize()[1]) {
            grad[7] = (size.grid(idx[0] + 1, idx[1] + 1) - size.grid(idx)) / dia;
        }
    }

    if (idx[1] + 1 < size.getGridSize()[1]) {
        grad[3] = (size.grid(idx[0], idx[1] + 1) - size.grid(idx)) / size.getCellSize()[1];
    }
}

static void checkLocalDifference(const SizeFunction& size, real_t limit) {
    int cnt = 0;

    for (std::size_t i = 0; i < size.getGridSize()[0] - 1; i++) {
        for (std::size_t j = 0; j < size.getGridSize()[1] - 1; j++) {

            vec2_t grad = forwardDifference(size, {i, j});
            if (std::abs(grad[0]) > limit*1.01 || std::abs(grad[1]) > limit*1.01) {
                cnt++;
            }
        }
    }
    std::cout << "Gradient errors: " << cnt << std::endl;
}

void simpleGradientLimiting(SizeFunction& size, real_t limit, real_t time_step, std::size_t iterations) {
    ScopeTimer timer("Simple gradient limiting");

    const size2_t grid_size = size.getGridSize();

    // create copy and swappable fields
    ScalarField<real_t> copy(size.getBoundingBox(), grid_size);
    copy.grid() = size.grid();
    ScalarField<real_t>* old_size = &copy;
    ScalarField<real_t>* new_size = &size;

    std::size_t iter = 0;
    std::size_t changed;

    do {
        changed = 0;

        // iterate over all points
        #pragma omp parallel for
        for (std::size_t i = 0; i < size.getGridSize()[0]; i++) {
            for (std::size_t j = 0; j < size.getGridSize()[1]; j++) {

                const size2_t idx(i, j);
                const real_t current = old_size->grid(idx);

                vec2_t max = backwardDifference(*old_size, idx);
                max[0] = std::max(max[0], 0.0);
                max[1] = std::max(max[1], 0.0);

                vec2_t min = forwardDifference(*old_size, idx);
                min[0] = std::min(min[0], 0.0);
                min[1] = std::min(min[1], 0.0);

                const real_t grad = std::sqrt(max.dot(max) + min.dot(min));

                if (grad > limit) {
                    new_size->grid(idx) = current + time_step * (limit - grad);

                    changed++;
                } else {
                    // no update needed
                    new_size->grid(idx) = current;
                }
            }
        }

        std::swap(old_size, new_size);
        iter++;
    } while (changed != 0 && iter < iterations);

    // swap data back to size if necessary
    if (old_size != &size) {
        size.grid() = std::move(old_size->grid());
    }

    checkLocalDifference(size, limit);
}


class MinHeap {  // TODO: check for errors
public:
    using Handle = std::size_t;  // index into heap

    explicit MinHeap(SizeFunction& size)
        : compare(size), size(size), heap(size.grid().size()), lookup_table(size.grid().size()) {

        const std::size_t heap_size = heap.size();

        // create heap
        for (std::size_t i = 0; i < heap_size; i++) {
            heap[i] = i;
        }
        std::make_heap(heap.begin(), heap.end(), compare);

        // create lookup
        for (std::size_t i = 0; i < heap_size; i++) {
            lookup_table[heap[i]] = i;
        }
    }

    size2_t pop() {
        const std::size_t idx = heap[0];

        // fix lookup
        lookup_table[idx] = heap.size();
        lookup_table[heap.back()] = 0;

        heap[0] = heap.back();
        heap.pop_back();
        sinkDown(0);

        return size.gridIndex(idx);
    }

    Handle find(const size2_t& idx) const {
        return lookup_table[size.linearIndex(idx)];
    }

    void update(Handle handle, real_t value) {
        const real_t old_value = size.grid()[heap[handle]];
        size.grid()[heap[handle]] = value;

        if (value > old_value) {
            sinkDown(handle);
        } else if (value < old_value) {
            bubbleUp(handle);
        }
    }

    bool isValid(Handle handle) const {
        return handle < heap.size();
    }

    bool empty() const {
        return heap.empty();
    }

private:
    void bubbleUp(Handle start) {
        while (start != 0) {
            const Handle parent = (start - 1) / 2;

            std::size_t& i_start = heap[start];
            std::size_t& i_parent = heap[parent];

            if (compare(i_parent, i_start)) {
                // swap with parent
                std::swap(i_start, i_parent);
                // fix lookup
                std::swap(lookup_table[i_start], lookup_table[i_parent]);
                start = parent;
            } else {
                break;
            }
        }
    }

    void sinkDown(Handle start) {
        while (true) {
            const Handle child0 = start * 2 + 1;
            const Handle child1 = child0 + 1;

            // get smallest child
            Handle target;
            if (isValid(child1)) {
                target = compare(heap[child0], heap[child1]) ? child1 : child0;
            } else if (isValid(child0)) {
                target = child0;
            } else {
                break;
            }

            std::size_t& i_start = heap[start];
            std::size_t& i_target = heap[target];

            if (compare(i_start, i_target)) {
                // swap with child
                std::swap(i_start, i_target);
                // fix lookup
                std::swap(lookup_table[i_start], lookup_table[i_target]);
                start = target;
            } else {
                break;
            }
        }
    }


    struct Comparator {
        explicit Comparator(const SizeFunction& size) : grid(size.grid()) {}

        bool operator()(std::size_t i0, std::size_t i1) const {
            return grid[i0] > grid[i1];
        }

        const std::vector<real_t>& grid;
    };
public:
    const Comparator compare;

    SizeFunction& size;

    std::vector<std::size_t> heap;  // stores linear index into grid_values
    std::vector<Handle> lookup_table;  // translates linear index to index into heap
};

void fastGradientLimitingAxial(SizeFunction& size, real_t limit, bool use_diagonals) {
    ScopeTimer timer("Fast gradient limiting axial");

    const vec2_t& cell_size = size.getCellSize();

    MinHeap heap(size);

    // iterate from lowest to highest point
    while (!heap.empty()) {

        const size2_t center = heap.pop();
        const real_t center_value = size.grid(center);

        std::vector<real_t> grad;  // gradient from center to neighbors
        std::vector<size2_t> neighbors;
        std::vector<real_t> spacing;

        if (!use_diagonals) {
            // only neighbors left, right, up, down
            const vec2_t backward = backwardDifference(size, center);
            const vec2_t forward = forwardDifference(size, center);

            grad = {backward[0], backward[1], forward[0], forward[1]};

            neighbors = {{center[0] - 1, center[1]}, {center[0], center[1] - 1},
                         {center[0] + 1, center[1]}, {center[0], center[1] + 1}};

            spacing = {cell_size[0], cell_size[1], cell_size[0], cell_size[1]};

        } else {
            // additionally use neighbors on diagonals
            grad.resize(8);
            allDifferences(size, center, grad);

            neighbors = {{center[0] - 1, center[1]}, {center[0], center[1] - 1},
                         {center[0] + 1, center[1]}, {center[0], center[1] + 1},
                         {center[0] - 1, center[1] - 1}, {center[0] - 1, center[1] + 1},
                         {center[0] + 1, center[1] - 1}, {center[0] + 1, center[1] + 1}};

            const real_t diagonal = cell_size.norm();
            spacing = {cell_size[0], cell_size[1], cell_size[0], cell_size[1],
                       diagonal, diagonal, diagonal, diagonal};
        }

        // iterate over neighbors
        for (std::size_t i = 0; i < neighbors.size(); i++) {

            if (std::abs(grad[i]) > limit) {  // also handles boundary check (grad[i] == 0)

                const MinHeap::Handle handle = heap.find(neighbors[i]);
                if (heap.isValid(handle)) {  // still in heap

                    const real_t new_size = (limit * spacing[i] + center_value) * 0.999999;

                    assert(new_size < size.grid(neighbors[i]) * 1.01);

                    heap.update(handle, new_size);  // also updates size function
                }
            }
        }
    }
    checkLocalDifference(size, limit);
}


static void solveQuadrant(std::list<real_t>& solutions, real_t v0, real_t v1, real_t param_c, real_t max_value) {
    // Consider the gradient in this quadrant as a combination
    // of the differences to the two neighbors on the axes of this quadrant.
    // The length of this gradient is set to be the limit.
    // This creates a quadratic equation that can be solved for the current points value.

    real_t a = 0;
    real_t b = 0;
    real_t c = -param_c;

    // only consider points with fixed values (less than currently smallest value in heap)
    if (v0 <= max_value) {
        a++;
        b += -2 * v0;
        c += v0 * v0;
    }
    if (v1 <= max_value) {
        a++;
        b += -2 * v1;
        c += v1 * v1;
    }

    if (a == 0) {
        return;
    }

    const real_t discriminant = b * b - 4 * a * c;

    // only positive solutions are valid
    if (discriminant == 0 && b <= 0) {
        solutions.push_back(-b / (2 * a));

    } else if (discriminant > 0) {
        solutions.push_back(std::abs((-b + std::sqrt(discriminant)) / (2 * a)));
    }
}

void fastGradientLimiting(SizeFunction& size, real_t limit) {
    ScopeTimer timer("Fast gradient limiting");

    const size2_t& grid_size = size.getGridSize();
    const real_t cell_size = size.getCellSize()[0];  // assume equal cell sizes in x and y
    const real_t param_c = limit * limit * cell_size * cell_size;  // parameter c in quadratic equation

    const int offset[4][2] = {{-1, 0}, {0, -1}, {1, 0}, {0, 1}};

    MinHeap heap(size);

    // iterate from lowest to highest point
    while (!heap.empty()) {

        const size2_t center = heap.pop();
        const real_t center_value = size.grid(center);

        // iterate over neighbors
        for (int i = 0; i < 4; i++) {

            // bounds checks
            if ((center[0] == 0 && offset[i][0] == -1) || (center[1] == 0 && offset[i][1] == -1)) {
                continue;
            }

            const size2_t idx(center[0] + offset[i][0], center[1] + offset[i][1]);

            if (idx[0] >= grid_size[0] || idx[1] >= grid_size[1]) {
                continue;
            }

            const MinHeap::Handle handle = heap.find(idx);
            if (heap.isValid(handle)) {  // still in heap

                vec2_t max = backwardDifference(size, idx);
                max[0] = std::max(max[0], 0.0);
                max[1] = std::max(max[1], 0.0);

                vec2_t min = forwardDifference(size, idx);
                min[0] = std::min(min[0], 0.0);
                min[1] = std::min(min[1], 0.0);

                const real_t grad = std::sqrt(max.dot(max) + min.dot(min));

                if (grad > limit) {

                    const real_t old_size = size.grid(idx);

                    // neighbors neighbors size values
                    const real_t nn0 = max[0] == 0 ? old_size : size.grid(idx[0] - 1, idx[1]);
                    const real_t nn1 = max[1] == 0 ? old_size : size.grid(idx[0], idx[1] - 1);
                    const real_t nn2 = min[0] == 0 ? old_size : size.grid(idx[0] + 1, idx[1]);
                    const real_t nn3 = min[1] == 0 ? old_size : size.grid(idx[0], idx[1] + 1);

                    std::list<real_t> solutions;
                    // consider each quadrant separately
                    solveQuadrant(solutions, nn0, nn1, param_c, center_value);
                    solveQuadrant(solutions, nn0, nn3, param_c, center_value);
                    solveQuadrant(solutions, nn2, nn1, param_c, center_value);
                    solveQuadrant(solutions, nn2, nn3, param_c, center_value);

                    // get smallest solution
                    real_t new_size = old_size;
                    for (real_t s : solutions) {
                        if (s < new_size) {
                            assert(s > 0);
                            new_size = s;
                        }
                    }

                    assert(new_size < size.grid(idx) * 1.01);

                    heap.update(handle, new_size);  // also updates size function
                }
            }
        }
    }
    checkLocalDifference(size, limit);
}


void jigsawGradientLimiting(SizeFunction& size, real_t limit) {
    ScopeTimer timer("Jigsaw gradient limiting");

    jigsaw_jig_t jig;
    jigsaw_init_jig_t(&jig);

    jig._verbosity = 1;

    JigsawSizeFunction h_fun(size);
    h_fun.setGradientLimit(limit);

    int retv = marche(&jig, &h_fun.getJigsawMesh());

    if (retv != 0) {
        throw std::runtime_error("jigsaw error " + std::to_string(retv));
    }

    h_fun.toSizeFunction(size);

    checkLocalDifference(size, limit);
}

}

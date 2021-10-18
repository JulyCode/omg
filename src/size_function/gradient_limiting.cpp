
#include "gradient_limiting.h"

#include <iostream>
#include <queue>
#include <set>

#include <io/vtk_writer.h>
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

static void check(const SizeFunction& size, real_t limit) {
    int cnt = 0;

    for (std::size_t i = 0; i < size.getGridSize()[0] - 1; i++) {
        for (std::size_t j = 0; j < size.getGridSize()[1] - 1; j++) {

            vec2_t grad = forwardDifference(size, {i, j});
            if (std::abs(grad[0]) > limit*1.01 || std::abs(grad[1]) > limit*1.01) {
                cnt++;
            }
        }
    }
    std::cout << "Errors: " << cnt << std::endl;
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
        std::cout << "iteration " << iter << " changed " << changed << std::endl;
        iter++;
    } while (changed != 0 && iter < iterations);

    // swap data back to size if necessary
    if (old_size != &size) {
        size.grid() = std::move(old_size->grid());
    }

    check(size, limit);
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

void fastGradientLimiting(SizeFunction& size, real_t limit) {
    ScopeTimer timer("Fast gradient limiting");

    const vec2_t& cell_size = size.getCellSize();
    limit /= std::sqrt(2);

    MinHeap heap(size);

    // iterate from lowest to highest point
    while (!heap.empty()) {

        if (heap.heap.size() % 1000000 == 0) {std::cout << heap.heap.size() << std::endl;}

        const size2_t center = heap.pop();
        const real_t center_value = size.grid(center);

        const vec2_t backward = backwardDifference(size, center);
        const vec2_t forward = forwardDifference(size, center);

        // gradient from center to neighbors
        const real_t grad[4] = {backward[0], backward[1], forward[0], forward[1]};

        const size2_t neighbors[4] = {{center[0] - 1, center[1]}, {center[0], center[1] - 1},
                                      {center[0] + 1, center[1]}, {center[0], center[1] + 1}};

        const real_t spacing[4] = {cell_size[0], cell_size[1], cell_size[0], cell_size[1]};

        // iterate over neighbors
        for (int i = 0; i < 4; i++) {

            if (std::abs(grad[i]) > limit) {  // also handles boundary check (grad[i] == 0)

                const MinHeap::Handle handle = heap.find(neighbors[i]);
                if (heap.isValid(handle)) {  // still in heap

                    const real_t new_size = limit * spacing[i] + center_value;  // TODO: small epsilon

                    assert(new_size < size.grid(neighbors[i]) * 1.01);

                    heap.update(handle, new_size);  // also updates size function
                }
            }
        }
    }
    check(size, limit);
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

    check(size, limit);
}

}

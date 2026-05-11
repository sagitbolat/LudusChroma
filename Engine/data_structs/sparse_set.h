#include <iostream>
#include <stdint.h>
#include<vector>
// A structure to hold the three parameters required to
// represent a sparse set.

// NOTE: id_t is usually an entity struct that hold entity id and generation
//       while data_t is usually the component struct
#define SPARSE_SET_NULL_INDEX -1
template<typename T>
struct SparseSet
{
    std::vector<int> sparse;   // To store indexes of actual elements
    std::vector<T>   dense = {};    // To store actual set elements
    std::vector<int> dense_ids;   // dense index -> id

    // Initialization
    void Init(int maxV = 100)
    {
        sparse.assign(maxV, SPARSE_SET_NULL_INDEX);
        dense.clear();
        dense_ids.clear();
    }

    // De-initialization
    void Free()
    {
        // TODO
    }

    // Inserts a new element into set
    void Insert(int id, T data) {
        if (id < 0) return;

        if (id >= (int)sparse.size())
            sparse.resize(id + 1, SPARSE_SET_NULL_INDEX);

        int& idx = sparse[id];

        if (idx != SPARSE_SET_NULL_INDEX) {
            // already exists: overwrite data
            dense[idx] = data;
            return;
        }

        idx = (int)dense.size();
        dense.push_back(data);
        dense_ids.push_back(id);
    }

    T* Get(int id) {
        if (id < 0 || id >= (int)sparse.size())
            return nullptr;

        int idx = sparse[id];
        if (idx == SPARSE_SET_NULL_INDEX)
            return nullptr;

        // Optional safety check (good during debugging):
        // if (dense_ids[idx] != id) return nullptr;

        return &dense[idx];
    }

    // Deletes an element
    //FIXME: now working for some reason
    void Delete(int id) {
        if (id < 0 || id >= (int)sparse.size())
            return;

        int idx = sparse[id];
        if (idx == SPARSE_SET_NULL_INDEX)
            return;

        int last = (int)dense.size() - 1;

        if (idx != last) {
            // move last element into the hole
            dense[idx] = std::move(dense[last]);

            int moved_id = dense_ids[last];
            dense_ids[idx] = moved_id;
            sparse[moved_id] = idx;
        }

        dense.pop_back();
        dense_ids.pop_back();
        sparse[id] = SPARSE_SET_NULL_INDEX;
    }

    // Removes all elements from set
    void Clear() {
        std::fill(sparse.begin(), sparse.end(), SPARSE_SET_NULL_INDEX);
        dense.clear();
        dense_ids.clear();
    }
};


/* TEST CODE
struct Entity{
    uint32_t id;
    uint32_t generation;
};

struct Component {
    int a;
    int b;
};


void print_vec(std::vector<int> v) {
    // Printing elements of vector
    std::cout << "sparse: ";
    for (auto i : v)
        std::cout << i << " ";
    std::cout << std::endl;
}

void print_vec(std::vector<Component> v) {

    // Printing elements of vector
    std::cout << "dense: ";
    for (auto i : v)
        std::cout << "{" << i.a << "," << i.b << "} ";
    std::cout << std::endl;
}

void print_set(SparseSet<Component> ss) {
    std::cout << "==============SET============" << "\n";
    print_vec(ss.dense);
    print_vec(ss.sparse);
    std::cout << "=============================" << "\n";

}

int main()
{
    SparseSet<Component> comp_arr;
    comp_arr.Init(3);

    print_set(comp_arr);

    comp_arr.Insert(int(0), Component{0, 0});
    comp_arr.Insert(int(1), Component{1, 1});
    comp_arr.Insert(int(5), Component{5, 1239});

    print_set(comp_arr);

    std::cout << "Element 3 Get: " << ((nullptr == comp_arr.Get(3)) ? "invalid" : "valid")<< std::endl;
    std::cout << "Element 2 Get: " << ((nullptr == comp_arr.Get(2)) ? "invalid" : "valid")<< std::endl;
    std::cout << "Element 2 Get: " << ((nullptr == comp_arr.Get(1)) ? "invalid" : "valid")<< std::endl;

    print_set(comp_arr);

    std::cout << "======Deleting========" << std::endl;
    comp_arr.Delete(1);
    print_set(comp_arr);
    comp_arr.Delete(1);
    print_set(comp_arr);
    comp_arr.Delete(0);
    print_set(comp_arr);
    comp_arr.Delete(5);
    print_set(comp_arr);
    comp_arr.Insert(int(0), Component{0, 0});
    comp_arr.Insert(int(1), Component{1, 1});
    comp_arr.Insert(int(5), Component{5, 1239});

    print_set(comp_arr);

    return 0;
}


*/

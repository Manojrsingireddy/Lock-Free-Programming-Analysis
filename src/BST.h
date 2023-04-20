#include <atomic>
#include <pthread.h>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>

using namespace std;

struct bst_node_struct{
    int key;
    int val;
    struct bst_node_struct * left;
    struct bst_node_struct * right;
    bst_node_struct(int k, int v){
        key = k;
        val = v;
        left = nullptr;
        right = nullptr;
    }
};
typedef struct bst_node_struct bst_node;

class lock_based_bst{
    public:
        lock_based_bst();
        ~lock_based_bst();
        void insert(int key, int val);
        int get(int key);
        void remove(int key);
    private:
        bst_node * root;
        pthread_mutex_t lock;
};


struct lf_bst_node_struct{
    int key;
    int val;
    atomic<struct lf_bst_node_struct *> left;
    atomic<struct lf_bst_node_struct *> right;

    lf_bst_node_struct(int k, int v)
        : key(k), val(v), left(nullptr), right(nullptr){}
};
typedef struct lf_bst_node_struct lf_bst_node;


class lock_free_bst{
    public:
        lock_free_bst();
        ~lock_free_bst();
        void insert(int key, int val);
        int get(int key);
    private:
        atomic<lf_bst_node *> root;
};

void performBSTTests(double readWriteRatio, int num_threads, int num_ops);
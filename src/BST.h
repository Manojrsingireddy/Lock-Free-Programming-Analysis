#include <atomic>
#include <pthread.h>
#include <iostream>

using namespace std;

struct bst_node_struct{
    int key;
    int val;
    struct bst_node_struct * left;
    struct bst_node_struct * right;
    bst_node_struct(int k, int v){
        key = k;
        val = v;
        left = NULL;
        right = NULL;
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

struct Info {
    enum class Type { Insert, Delete } type;
    lf_bst_node *old_node;
    lf_bst_node *new_node;
    atomic<Info *> parent_info;

    Info(Type t, lf_bst_node *old_n, lf_bst_node *new_n)
        : type(t), old_node(old_n), new_node(new_n), parent_info(nullptr) {}
};


struct lf_bst_node_struct{
    int key;
    int val;
    atomic<struct lf_bst_node_struct *> left;
    atomic<struct lf_bst_node_struct *> right;
    atomic<Info *> op_info;

    lf_bst_node_struct(int k, int v)
        : key(k), val(v), left(nullptr), right(nullptr), op_info(nullptr) {}
};
typedef struct lf_bst_node_struct lf_bst_node;

class lock_free_bst{
    public:
        lock_free_bst();
        ~lock_free_bst();
        void insert(int key, int val);
        int get(int key);
        void remove(int key);
    private:
        atomic<lf_bst_node *> root;
};
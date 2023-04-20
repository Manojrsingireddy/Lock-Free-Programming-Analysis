#include <pthread.h>
#include <atomic>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>

#define BAD_VAL -1

using namespace std;

enum stacktype{
    lock_free,
    lock_based,
};

template <typename T>
struct Node_struct{
    T val;
    struct Node_struct * next;
    Node_struct(int val){
        this->val = val;
        this->next = NULL;
    }
};

typedef struct Node_struct<int> Node;


class myStack{
    private:
    atomic<Node *> atom_head;
    Node * head;
    stacktype type;
    pthread_mutex_t lock;
    public:
    myStack(stacktype s): head(nullptr), type(s){}
    void push(int val);
    int pop();
};

void performStackTests(double readWriteRatio, int num_threads, int num_ops);
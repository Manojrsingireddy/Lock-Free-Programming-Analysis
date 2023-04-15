#include <pthread.h>
#include <atomic>

#define BAD_VAL -1

using namespace std;

enum stacktype{
    lock_free,
    lock_based,
    single_threaded
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


class Stack{
    private:
    atomic<Node *> head;
    stacktype type;
    pthread_mutex_t lock;
    public:
    Stack(stacktype s): head(nullptr), type(s){}
    void push(int val);
    int pop();
};


void run_stack_tests();
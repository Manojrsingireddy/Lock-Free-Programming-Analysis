#include "stack.h"

using namespace std;

int myStack::pop(){
    int val = 0;
    Node * oldHead;
    switch(type) {
        case lock_free:
            oldHead = atom_head.load(memory_order_relaxed);
            while (oldHead && !atom_head.compare_exchange_weak(oldHead, oldHead->next, memory_order_release, memory_order_relaxed)) {}
            if (oldHead) {
                val = oldHead->val;
                // delete oldHead;
            }
            else{
                val = BAD_VAL;
            }
            break;
        case lock_based:
            pthread_mutex_lock(&lock);
            oldHead = head;
            if (oldHead) {
                val = oldHead->val;
                head = oldHead->next;
                // delete oldHead;
            }
            else{
                val = BAD_VAL;
            }
            pthread_mutex_unlock(&lock);
            break;          
    }
    return val;
}

void myStack::push(int val){
    Node * newNode = new Node(val);
    switch(type){
        case lock_free:
            newNode->next = atom_head.load(memory_order_relaxed);
            while (!atom_head.compare_exchange_weak(newNode->next, newNode, memory_order_release, memory_order_relaxed)) {}
            break;
        case lock_based:
            pthread_mutex_lock(&lock);
            newNode->next = head;
            head = newNode;
            pthread_mutex_unlock(&lock);
            break;
    }
}


struct tArgs_struct {
    void *Data_structure;
    double readWriteRatio;
    int num_ops;
    tArgs_struct() : Data_structure(nullptr), readWriteRatio(0), num_ops(0) {}
    tArgs_struct(void *ds, double rWR, int NO) : Data_structure(ds), readWriteRatio(rWR), num_ops(NO) {}
};
typedef struct tArgs_struct tArgs;


void * performStackOperations(void * args){
    tArgs *threadArgs = static_cast<tArgs *>(args);
    myStack *stack = (myStack *) threadArgs->Data_structure;
    double readWriteRatio = threadArgs->readWriteRatio;
    int num_ops = threadArgs->num_ops;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);

    for (int i = 0; i < num_ops; ++i) {
        double random_value = dis(gen);
        if (random_value <= readWriteRatio) {
            stack->pop();
        } else {
            stack->push(i);
        }
    }
    return nullptr;
}

long long performAStackTest(double readWriteRatio, int num_threads, int num_ops, stacktype st){
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    myStack stack(st);
    std::vector<pthread_t> threads(num_threads);
    std::vector<tArgs> threadArgs(num_threads);
    for(int i = 0; i < num_threads; ++i){
        threadArgs[i] = tArgs((void *) &stack, readWriteRatio, num_ops);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, performStackOperations, &threadArgs[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    std::__1::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


void performStackTests(double readWriteRatio, int num_threads, int num_ops){
    cout << "Performing stack tests..." << endl;
    long long lb_time = performAStackTest(readWriteRatio, num_threads, num_ops, lock_based);
    long long lf_time = performAStackTest(readWriteRatio, num_threads, num_ops, lock_free);

    // Summary statictics
    std::cout << "Lock based stack time: " << lb_time << " nanoseconds" << std::endl;
    std::cout << "Lock free stack time: " << lf_time << " nanoseconds" << std::endl;
    std::cout << "Speedup: " << ((double)lb_time / (double)lf_time) << std::endl;

}
#include "stack.h"

using namespace std;

int Stack::pop(){
    int val = 0;
    Node * oldHead;
    switch(type) {
        case lock_free:
            oldHead = head.load(memory_order_relaxed);
            while (oldHead && !head.compare_exchange_weak(oldHead, oldHead->next, memory_order_release, memory_order_relaxed)) {}
            if (oldHead) {
                val = oldHead->val;
                delete oldHead;
            }
            else{
                val = BAD_VAL;
            }
            break;
        case lock_based:
            pthread_mutex_lock(&lock);
            oldHead = head.load(memory_order_relaxed);
            if (oldHead) {
                val = oldHead->val;
                head.store(oldHead->next);
                delete oldHead;
            }
            else{
                val = BAD_VAL;
            }
            pthread_mutex_unlock(&lock);
            break;
        case single_threaded:
            oldHead = head.load(memory_order_relaxed);
            if (oldHead) {
                val = oldHead->val;
                head.store(oldHead->next);
                delete oldHead;
            }
            else{
                val = BAD_VAL;
            }
            break;            
    }
    return val;
}

void Stack::push(int val){
    Node * newNode = new Node(val);
    switch(type){
        case lock_free:
            newNode->next = head.load(memory_order_relaxed);
            while (!head.compare_exchange_weak(newNode->next, newNode, memory_order_release, memory_order_relaxed)) {}
            break;
        case lock_based:
            pthread_mutex_lock(&lock);
            newNode->next = head.load(memory_order_relaxed);
            head.store(newNode);
            pthread_mutex_unlock(&lock);
            break;
        case single_threaded:
            newNode->next = head.load(memory_order_relaxed);
            head.store(newNode);
            break;
    }
}

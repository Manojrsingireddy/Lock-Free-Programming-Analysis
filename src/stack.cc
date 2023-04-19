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

#include "BST.h"
#define BAD_VAL -1
using namespace std;

lock_based_bst::lock_based_bst(){
    root = NULL;
    pthread_mutex_init(&lock, NULL);
}

lock_based_bst::~lock_based_bst(){
    pthread_mutex_destroy(&lock);
}

int lock_based_bst::get(int key){
    pthread_mutex_lock(&lock);
    bst_node * curr = root;
    int val = BAD_VAL;
    while(curr != NULL){
        if(curr->key == key){
            val = curr->val;
            break;
        }
        else if(curr->key > key){
            curr = curr->left;
        }
        else{
            curr = curr->right;
        }
    }
    pthread_mutex_unlock(&lock);
    return val;
}

void lock_based_bst::insert(int key, int val){
    pthread_mutex_lock(&lock);
    if(root == nullptr){
        root = new bst_node(key, val);
        pthread_mutex_unlock(&lock);
        return;
    }
    bst_node * curr = root;
    bst_node * prev = nullptr;
    while(curr){
        prev = curr;
        if(curr->key > key){
            curr = curr->left;
        }
        else{
            curr = curr->right;
        }
    }
    if(key < prev->key){
        prev->left = new bst_node(key, val);
    }
    else{
        prev->right = new bst_node(key, val);
    }
    pthread_mutex_unlock(&lock);    
}

void lock_based_bst::remove(int key){
    pthread_mutex_lock(&lock);
    bst_node * curr = root;
    bst_node * prev = nullptr;
    while(curr){
        if(curr->key == key){
            break;
        }
        prev = curr;
        if(curr->key > key){
            curr = curr->left;
        }
        else{
            curr = curr->right;
        }
    }
    if(curr == nullptr){
        pthread_mutex_unlock(&lock);
        return;
    }
    if(curr->left == nullptr && curr->right == nullptr){
        if(prev == nullptr){
            root = nullptr;
        }
        else if(prev->left == curr){
            prev->left = nullptr;
        }
        else{
            prev->right = nullptr;
        }
        delete curr;
    }
    else if(curr->left == nullptr){
        if(prev == nullptr){
            root = curr->right;
        }
        else if(prev->left == curr){
            prev->left = curr->right;
        }
        else{
            prev->right = curr->right;
        }
        delete curr;
    }
    else if(curr->right == nullptr){
        if(prev == nullptr){
            root = curr->left;
        }
        else if(prev->left == curr){
            prev->left = curr->left;
        }
        else{
            prev->right = curr->left;
        }
        delete curr;
    }
    else{
        bst_node * succ = curr->right;
        bst_node * succ_prev = curr;
        while(succ->left){
            succ_prev = succ;
            succ = succ->left;
        }
        curr->key = succ->key;
        curr->val = succ->val;
        if(succ_prev->left == succ){
            succ_prev->left = succ->right;
        }
        else{
            succ_prev->right = succ->right;
        }
        delete succ;
    }
    pthread_mutex_unlock(&lock);
}


void lock_free_bst::insert(int key, int val){
    lf_bst_node * new_node = new lf_bst_node(key, val);
        new_node->status.store(2); // Mark the node as newly added
        while (true) {
            lf_bst_node * curr = root;
            while (true) {
                // If the tree is empty, add the new node as root
                if (curr == nullptr) {
                    if (root.compare_exchange_strong(curr, new_node)) {
                        return;
                    } else {
                        break;
                    }
                }
                // If the key already exists, return false
                if (curr->key == key && curr->status.load() == 0) {
                    return;
                }
                // Traverse the tree to find the correct position to add the new node
                lf_bst_node * next = (key < curr->key) ? curr->left.load() : curr->right.load();
                if (next == nullptr) {
                    if ((key < curr->key && curr->left.compare_exchange_strong(next, new_node)) ||
                        (key > curr->key && curr->right.compare_exchange_strong(next, new_node))) {
                        return;
                    } else {
                        break;
                    }
                }
                curr = next;
            }
        }
}

void lock_free_bst::remove(int key){
    while (true) {
        lf_bst_node* curr = root;
        lf_bst_node* parent = nullptr;
        lf_bst_node* node_to_delete = nullptr;
        while (true) {
            // If the tree is empty, return false
            if (curr == nullptr) {
                return;
            }
            // If the key matches and the node is not already marked for deletion, mark it for deletion
            if (curr->key == key && curr->status.load() == 0) {
                if (!curr->status.compare_exchange_strong(0, 1)) {
                    break;
                }
                node_to_delete = curr;
            }
            // Traverse the tree to find the node to delete
            lf_bst_node* next = (key < curr->key) ? curr->left.load() : curr->right.load();
            if (next == nullptr) {
                break;
            }
            parent = curr;
            curr = next;
        }
        // If the node to delete was not found or was already deleted, return false
        if (node_to_delete == nullptr) {
            return;
        }
        // Remove the node from the tree by updating the parent's child pointer
        if (parent != nullptr) {
            lf_bst_node* child = (curr->left.load() == node_to_delete) ? curr->right.load() : curr->left.load();
            if (curr->key < parent->key) {
                parent->left.compare_exchange_strong(curr, child);
            } else {
                parent->right.compare_exchange_strong(curr, child);
            }
        } else {
            // If the root node is the node to delete, replace it with its successor
            lf_bst_node* successor = find_successor(curr);
            if (successor != nullptr) {
                successor->left.store(curr->left.load());
                successor->right.store(curr->right.load());
                while (!successor->status.compare_exchange_strong(2, 0)) {}
                if (!root.compare_exchange_strong(curr, successor)) {
                    return;
                }
            } else {
                if (!root.compare_exchange_strong(curr, nullptr)) {
                    return;
                }
            }
        }
        // Free the memory occupied by the node to delete
        delete node_to_delete;
        return;
    }
}

int lock_free_bst::get(int key){
    Node<Key, Value>* curr = root;
    while (curr != nullptr) {
        if (curr->key == key && curr->status.load() == 0) {
            value = curr->value;
            return true;
        }
        curr = (key < curr->key) ? curr->left.load() : curr->right.load();
    }
    return false;
}
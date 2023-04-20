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
    while(curr != nullptr){
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
    while(curr != nullptr){
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
    while(curr != nullptr){
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

lock_free_bst::lock_free_bst(){
    root.store(nullptr);
}

void delete_subtree(lf_bst_node *node) {
    if (node) {
        delete_subtree(node->left.load());
        delete_subtree(node->right.load());
        delete node;
    }
}

lock_free_bst::~lock_free_bst() {
    delete_subtree(root.load());
}


int lock_free_bst::get(int key){
    lf_bst_node * curr = root.load();
    while(curr){
        if(curr->key == key){
            return curr->val;
        }
        else if(curr->key > key){
            curr = curr->left.load();
        }
        else{
            curr = curr->right.load();
        }
    }
    return BAD_VAL;
}
void lock_free_bst::insert(int key, int val) {
    lf_bst_node *new_node = new lf_bst_node(key, val);
    lf_bst_node *cur = root.load();
    
    while (true) {
        if (cur == nullptr) {
            if (root.compare_exchange_weak(cur, new_node)) {
                return;
            }
        } else if (key < cur->key) {
            lf_bst_node *left = cur->left.load();
            if (left == nullptr) {
                if (cur->left.compare_exchange_weak(left, new_node)) {
                    return;
                }
            } else {
                cur = left;
            }
        } else if (key > cur->key) {
            lf_bst_node *right = cur->right.load();
            if (right == nullptr) {
                if (cur->right.compare_exchange_weak(right, new_node)) {
                    return;
                }
            } else {
                cur = right;
            }
        } else {
            // Overwrite value if the key already exists
            cur->val = val;
            delete new_node;
            return;
        }
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

void * performLockFreeBSTOperations(void * args){
    tArgs *threadArgs = static_cast<tArgs *>(args);
    lock_free_bst *bst = (lock_free_bst *) threadArgs->Data_structure;
    double readWriteRatio = threadArgs->readWriteRatio;
    int num_ops = threadArgs->num_ops;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    vector<int> keys(num_ops);

    for (int i = 0; i < num_ops; ++i) {
        double random_value = dis(gen);
        if (random_value <= readWriteRatio) {            
            int key = abs(rand()) % keys.size() - 1;
            bst->get(keys[key]);
            // bst->get(i);
        } else {            
            int key = abs(rand());
            keys.push_back(key);
            bst->insert(key, 0);
            // bst->insert(i, 0);
        }
    }
    return nullptr;
}

long long performLockFreeBSTTest(double readWriteRatio, int num_threads, int num_ops){
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    // Create bst
    lock_free_bst * bst = new lock_free_bst();
    std::vector<pthread_t> threads(num_threads);
    std::vector<tArgs> threadArgs(num_threads);
    for(int i = 0; i < num_threads; ++i){
        threadArgs[i] = tArgs((void *) bst, readWriteRatio, num_ops);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, performLockFreeBSTOperations, &threadArgs[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    std::__1::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    delete bst;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}



void * performLockBasedBSTOperations(void * args){
    tArgs *threadArgs = static_cast<tArgs *>(args);
    lock_based_bst *bst = (lock_based_bst *) threadArgs->Data_structure;
    double readWriteRatio = threadArgs->readWriteRatio;
    int num_ops = threadArgs->num_ops;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
    vector<int> keys(num_ops);

    for (int i = 0; i < num_ops; ++i) {
        double random_value = dis(gen);
        if (random_value <= readWriteRatio) {            
            int key = abs(rand()) % keys.size() - 1;
            bst->get(keys[key]);
            // bst->get(i);
        } else {            
            int key = abs(rand());
            keys.push_back(key);
            bst->insert(key, 0);
            // bst->insert(i, 0);
        }
    }
    return nullptr;
}

long long performLockBasedBSTTest(double readWriteRatio, int num_threads, int num_ops){
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    // Create the bst
    lock_based_bst * bst = new lock_based_bst();
    std::vector<pthread_t> threads(num_threads);
    std::vector<tArgs> threadArgs(num_threads);
    for(int i = 0; i < num_threads; ++i){
        threadArgs[i] = tArgs((void *) bst, readWriteRatio, num_ops);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, performLockBasedBSTOperations, &threadArgs[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    delete bst;
    std::__1::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


void performBSTTests(double readWriteRatio, int num_threads, int num_ops){
    cout << "Performing BST tests..." << endl;
    long long lb_time = performLockBasedBSTTest(readWriteRatio, num_threads, num_ops);
    long long lf_time = performLockFreeBSTTest(readWriteRatio, num_threads, num_ops);
    // Summary statictics
    std::cout << "Lock based hash table time: " << lb_time << " nanoseconds" << std::endl;
    std::cout << "Lock free hash table time: " << lf_time << " nanoseconds" << std::endl;
    std::cout << "Speedup: " << ((double)lb_time / (double)lf_time) << std::endl;
}

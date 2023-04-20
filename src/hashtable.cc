#include "hashtable.h"

#define BAD_VALUE -1

using namespace std;

static inline uint32_t make_hash(uint32_t key, uint32_t size){ // MurmurHash3 integer finalizer
    uint32_t h = key;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h % size;
}

lock_based_hash_table::lock_based_hash_table(int s): size(s), table(s){
    pthread_mutex_init(&lock, NULL);
}

lock_based_hash_table::~lock_based_hash_table(){
    pthread_mutex_destroy(&lock);
}

void lock_based_hash_table::insert(int key, int value) {
    pthread_mutex_lock(&lock);
    int index = make_hash(key, size);
    for (const auto& entry : table[index]) {
        if (entry.first == key) {
            pthread_mutex_unlock(&lock);
            return;
        }
    }
    std::pair<int, int> new_entry(key, value);
    table[index].push_back(new_entry);
    pthread_mutex_unlock(&lock);
}

int lock_based_hash_table::get(int key) {
    pthread_mutex_lock(&lock);
    int index = make_hash(key, size);
    for (const auto& entry : table[index]) {
        if (entry.first == key) {
            pthread_mutex_unlock(&lock);
            return entry.second;
        }
    }
    pthread_mutex_unlock(&lock);
    return BAD_VALUE; // Return a default value or throw an exception if the key is not found
}

void lock_based_hash_table::remove(int key) {
    pthread_mutex_lock(&lock);
    int index = make_hash(key,size);
    auto& bucket = table[index];
    for (auto it = bucket.begin(); it != bucket.end(); ++it) {
        if (it->first == key) {
            bucket.erase(it);
            break;
        }
    }
    pthread_mutex_unlock(&lock);
}


lock_free_hash_table::lock_free_hash_table(int s): size(s), table(s){
    for (size_t i = 0; i < size; i++) {
        table[i].store(nullptr);
    }
}

lock_free_hash_table::~lock_free_hash_table(){
    for (size_t i = 0; i < size; ++i) {
        htEntry* current = table[i].load();
        while (current) {
            htEntry* next = current->next;
            delete current;
            current = next;
        }
    }
}



void lock_free_hash_table::insert(int key, int value) {
    size_t index = make_hash(key, size);
    htEntry* new_node = new htEntry(key, value);
    htEntry* expected = table[index].load(std::memory_order_relaxed);

    while (true) {
        htEntry* current = expected;
        while (current) {
            if (current->key == key) {
                delete new_node;
                return;
            }
            current = current->next;
        }

        new_node->next.store(expected, std::memory_order_relaxed);
        if (table[index].compare_exchange_weak(expected, new_node, std::memory_order_release, std::memory_order_relaxed)) {
            return;
        }
    }
}


int lock_free_hash_table::get(int key){
    size_t index = make_hash(key, size);
    int val = BAD_VALUE;
    htEntry* current = table[index].load();

    while (current) {
        if (current->key == key) {
            val = current->value;
            break;
        }
        current = current->next;
    }

    return val;
}

void lock_free_hash_table::remove(int key){
    size_t index = make_hash(key, size);
    htEntry* current = table[index].load();

    while (current) {
        if (current->key == key) {
            htEntry* next = current->next;
            if (table[index].compare_exchange_strong(current, next)) {
                delete current;
            }
        }
        current = current->next;
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

void * performLockFreeHTOperations(void * args){
    tArgs *threadArgs = static_cast<tArgs *>(args);
    lock_free_hash_table *HT = (lock_free_hash_table *) threadArgs->Data_structure;
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
            HT->get(keys[key]);
        } else {
            int key = abs(rand());
            keys.push_back(key);
            HT->insert(key, 0);
        }
    }
    return nullptr;
}

long long performLockFreeHTTest(double readWriteRatio, int num_threads, int num_ops){
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    // Create the hash table
    lock_free_hash_table ht(num_threads * num_ops);
    std::vector<pthread_t> threads(num_threads);
    std::vector<tArgs> threadArgs(num_threads);
    for(int i = 0; i < num_threads; ++i){
        threadArgs[i] = tArgs((void *) &ht, readWriteRatio, num_ops);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, performLockFreeHTOperations, &threadArgs[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    std::__1::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}

void * performLockBasedHTOperations(void * args){
    tArgs *threadArgs = static_cast<tArgs *>(args);
    lock_based_hash_table *HT = (lock_based_hash_table *) threadArgs->Data_structure;
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
            HT->get(keys[key]);
        } else {            
            int key = abs(rand());
            keys.push_back(key);
            HT->insert(key, 0);
        }
    }
    return nullptr;
}

long long performLockBasedHTTest(double readWriteRatio, int num_threads, int num_ops){
    std::__1::chrono::steady_clock::time_point start = std::chrono::high_resolution_clock::now();
    // Create the hash table
    lock_based_hash_table ht(num_threads * num_ops);
    std::vector<pthread_t> threads(num_threads);
    std::vector<tArgs> threadArgs(num_threads);
    for(int i = 0; i < num_threads; ++i){
        threadArgs[i] = tArgs((void *) &ht, readWriteRatio, num_ops);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_create(&threads[i], nullptr, performLockBasedHTOperations, &threadArgs[i]);
    }
    for (int i = 0; i < num_threads; ++i) {
        pthread_join(threads[i], nullptr);
    }
    std::__1::chrono::steady_clock::time_point end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
}


void performHashTests(double readWriteRatio, int num_threads, int num_ops){
    cout << "Performing hash table tests..." << endl;
    long long lb_time = performLockBasedHTTest(readWriteRatio, num_threads, num_ops);
    long long lf_time = performLockFreeHTTest(readWriteRatio, num_threads, num_ops);
    // Summary statictics
    std::cout << "Lock based hash table time: " << lb_time << " nanoseconds" << std::endl;
    std::cout << "Lock free hash table time: " << lf_time << " nanoseconds" << std::endl;
    std::cout << "Speedup: " << ((double)lb_time / (double)lf_time) << std::endl;
}
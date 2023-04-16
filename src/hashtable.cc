#include "hashtable.h"

#define BAD_VALUE -1

using namespace std;

static inline uint32_t make_hash(uint32_t key){ // MurmurHash3 integer finalizer
    uint32_t h = key;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}


single_threaded_hash_table::single_threaded_hash_table(int s){
    table.resize(size, make_pair(BAD_VALUE, BAD_VALUE));
    size = s;
}

single_threaded_hash_table::~single_threaded_hash_table(){
    table.clear();
}

void single_threaded_hash_table::insert(int key, int value){
    uint32_t index = make_hash(key);
    while(table[index].first != BAD_VALUE && table[index].first != key){
        index = (index + 1) % size;
    }
    table[index] = make_pair(key, value);
}

void single_threaded_hash_table::remove(int key){
    uint32_t index = make_hash(key);
    while(table[index].first != BAD_VALUE && table[index].first != key){
        index = (index + 1) % size;
    }
    table[index] = make_pair(BAD_VALUE, BAD_VALUE);
}

int single_threaded_hash_table::get(int key){
    uint32_t index = make_hash(key);
    while(table[index].first != BAD_VALUE && table[index].first != key){
        index = (index + 1) % size;
    }
    return table[index].second;
}

lock_based_hash_table::lock_based_hash_table(int s){
    table.resize(size, make_pair(BAD_VALUE, BAD_VALUE));
    size = s;
    pthread_mutex_init(&lock, NULL);
}

lock_based_hash_table::~lock_based_hash_table(){
    table.clear();
    pthread_mutex_destroy(&lock);
}

void lock_based_hash_table::insert(int key, int value){
    uint32_t index = make_hash(key);
    pthread_mutex_lock(&lock);
    while(table[index].first != BAD_VALUE && table[index].first != key){
        index = (index + 1) % size;
    }
    table[index] = make_pair(key, value);
    pthread_mutex_unlock(&lock);
}

void lock_based_hash_table::remove(int key){
    uint32_t index = make_hash(key);
    pthread_mutex_lock(&lock);
    while(table[index].first != BAD_VALUE && table[index].first != key){
        index = (index + 1) % size;
    }
    table[index] = make_pair(BAD_VALUE, BAD_VALUE);
    pthread_mutex_unlock(&lock);
}

int lock_based_hash_table::get(int key){
    uint32_t index = make_hash(key);
    pthread_mutex_lock(&lock);
    while(table[index].first != BAD_VALUE && table[index].first != key){
        index = (index + 1) % size;
    }
    int val = table[index].second;
    pthread_mutex_unlock(&lock);
    return val;
}

lock_free_hash_table::lock_free_hash_table(int s){
    size = s;
    ht_entries = new htEntry[size];
}

lock_free_hash_table::~lock_free_hash_table(){
    delete[] ht_entries;
}



void lock_free_hash_table::set(int key, int value){
    uint32_t idx = make_hash(key);
    while(true){
        idx &= size - 1;
        int check_key = ht_entries[idx].key.load(memory_order_relaxed);
        if(check_key == key){
            ht_entries[idx].value.store(value, memory_order_relaxed);
            return;
        }
        else if(check_key == 0){
            if(ht_entries[idx].key.compare_exchange_weak(check_key, key, memory_order_release, memory_order_relaxed)){
                ht_entries[idx].value.store(value, memory_order_relaxed);
                return;
            }
        }
        idx++;
    }
}

int lock_free_hash_table::get(int key){
    uint32_t idx = make_hash(key);
    while(true){
        idx &= size - 1;
        int check_key = ht_entries[idx].key.load(memory_order_relaxed);
        if(check_key == key){
            return ht_entries[idx].value.load(memory_order_relaxed);
        }
        else if(check_key == 0){
            return BAD_VALUE;
        }
        idx++;
    }
}

void lock_free_hash_table::remove(int key){
    uint32_t idx = make_hash(key);
    while(true){
        idx &= size - 1;
        int check_key = ht_entries[idx].key.load(memory_order_relaxed);
        if(check_key == key){
            ht_entries[idx].key.store(0, memory_order_relaxed);
            return;
        }
        else if(check_key == 0){
            return;
        }
        idx++;
    }
}
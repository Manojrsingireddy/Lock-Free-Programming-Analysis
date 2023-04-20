#include <pthread.h>
#include <atomic>
#include <iostream>
#include <random>
#include <vector>
#include <chrono>
#include <list>


using namespace std;

// linear probing based hash table

class lock_based_hash_table{
    public:
        lock_based_hash_table(int s);
        ~lock_based_hash_table();
        void insert(int key, int value);
        int get(int key);
        void remove(int key);
    private:
        int size;
        vector< list< pair<int, int> > > table;
        pthread_mutex_t lock;
};

struct htEntry_struct{
    int key;
    int value;
    atomic<htEntry_struct*> next;
    htEntry_struct(int k, int v): key(k), value(v), next(nullptr){};
};

typedef struct htEntry_struct htEntry;


class lock_free_hash_table{
    public:
        lock_free_hash_table(int s);
        ~lock_free_hash_table();
        void insert(int key, int value);
        int get(int key);
        void remove(int key);
    private:
        size_t size;
        vector<atomic<htEntry*> > table;
};

void performHashTests(double readWriteRatio, int num_threads, int num_ops);
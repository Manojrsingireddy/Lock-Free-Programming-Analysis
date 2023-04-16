#include <pthread.h>
#include <atomic>
#include <iostream>


using namespace std;

// linear probing based hash table

class single_threaded_hash_table{
    public:
        single_threaded_hash_table(int s);
        ~single_threaded_hash_table();
        void insert(int key, int value);
        int get(int key);
        void remove(int key);
    private:
        int size;
        vector< pair<int, int> > table;
};

class lock_based_hash_table{
    public:
        lock_based_hash_table(int s);
        ~lock_based_hash_table();
        void insert(int key, int value);
        int get(int key);
        void remove(int key);
    private:
        int size;
        vector< pair<int, int> > table;
        pthread_mutex_t lock;
};

struct htEntry_struct{
    atomic<int> key;
    atomic<int> value;
};

typedef struct htEntry_struct htEntry;


class lock_free_hash_table{
    public:
        lock_free_hash_table(int s);
        ~lock_free_hash_table();
        void set(int key, int value);
        int get(int key);
        void remove(int key);
    private:
        int size;
        htEntry * ht_entries;
};

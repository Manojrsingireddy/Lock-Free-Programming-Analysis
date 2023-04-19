#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <random>
#include <pthread.h>
#include <vector>
#include <chrono>
#include "stack.h"
#include "hashtable.h"
// #include "BST.h"

using namespace std;

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
    cout << "Performing stack tests..." << std::endl;
    long long lb_time = performAStackTest(readWriteRatio, num_threads, num_ops, lock_based);
    long long lf_time = performAStackTest(readWriteRatio, num_threads, num_ops, lock_free);

    // Summary statictics
    std::cout << "Lock based stack time: " << lb_time << " nanoseconds" << std::endl;
    std::cout << "Lock free stack time: " << lf_time << " nanoseconds" << std::endl;
    std::cout << "Speedup: " << ((double)lb_time / (double)lf_time) << std::endl;

}

void performHashTests(double readWriteRatio, int num_threads, int num_ops) {
    std::cout << "Performing hash tests..." << std::endl;
}

void performBSTTests(double readWriteRatio, int num_threads, int num_ops) {
    std::cout << "Performing BST tests..." << std::endl;
}

int main(int argc, char *argv[]) {
    int opt;
    double readWriteRatio = -1;
    bool do_stack_tests = false;
    bool do_hash_tests = false;
    bool do_bst_tests = false;
    int num_threads = -1;
    int num_ops = -1;

    while ((opt = getopt(argc, argv, "sht:r:n:o:")) != -1) {
        switch (opt) {
            case 's':
                do_stack_tests = true;
                break;
            case 'h':
                do_hash_tests = true;
                break;
            case 't':
                do_bst_tests = true;
                break;
            case 'r':
                readWriteRatio = std::atof(optarg);
                break;
            case 'n':
                num_threads = std::atoi(optarg);
                break;
            case 'o':
                num_ops = std::atoi(optarg);
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-s] [-h] [-t] [-r ratio] [-n num_threads] [-o num_ops]" << std::endl;
                return 1;
        }
    }
    // print argument values
    // cout << "do_stack_tests: " << do_stack_tests << endl;
    // cout << "do_hash_tests: " << do_hash_tests << endl;
    // cout << "do_bst_tests: " << do_bst_tests << endl;
    // cout << "readWriteRatio: " << readWriteRatio << endl;
    // cout << "num_threads: " << num_threads << endl;
    // cout << "num_ops: " << num_ops << endl;

    if (readWriteRatio < 0 || readWriteRatio > 1) {
        std::cerr << "Error: -r option with read/write ratio is required and must be in [0,1]." << std::endl;
        return 1;
    }
    if (num_threads < -1) {
        std::cerr << "Error: -n option with num_threads is required." << std::endl;
        return 1;
    }
    if (num_ops < -1) {
        std::cerr << "Error: -o option with num_ops is required." << std::endl;
        return 1;
    }

    if(do_stack_tests) {
        performStackTests(readWriteRatio, num_threads, num_ops);
    }
    if(do_hash_tests) {
        performHashTests(readWriteRatio, num_threads, num_ops);
    }
    if(do_bst_tests) {
        performBSTTests(readWriteRatio, num_threads, num_ops);
    }

    return 0;
}

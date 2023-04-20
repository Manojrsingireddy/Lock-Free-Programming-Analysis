#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include "stack.h"
#include "hashtable.h"
#include "BST.h"

#define SEED 88884444

using namespace std;

int main(int argc, char *argv[]) {
    int opt;
    double readWriteRatio = -1;
    bool do_stack_tests = false;
    bool do_hash_tests = false;
    bool do_bst_tests = false;
    int num_threads = -1;
    int num_ops = -1;

    while ((opt = getopt(argc, argv, "shtr:n:o:")) != -1) {
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
    // // print argument values
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
    srand(SEED);

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

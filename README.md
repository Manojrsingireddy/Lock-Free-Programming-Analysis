# Lock-Fre Programing Analysis
## Description
In this project, I have implemented various lock-free data structures in C++ and analyzed their performance. The implementations are located in the src directory. The data structures implemented are:
* Lock-Free Stack
* Lock-Free Hash Table
* Lock-Free Binary Search Tree

## How to Run
To run the program, you must have g++ installed. Then, you can run the following commands:
```
make
./run [-s] [-h] [-t] [-r ratio] [-n num_threads] [-o num_ops]
```
The arguments are as follows:
* -s: Run the stack test (one of the three test types is required)
* -h: Run the hash table test
* -t: Run the binary search tree test
* -r: The ratio of insert operations to remove operations (required)
* -n: The number of threads to run (required)
* -o: The number of operations to run (required)

Another option is to run the following command:
```
python3 test_run.py
```
OR
```
python3 test_run.py
'''

## Results
The results of the testing is located in the Results.pdf file.
import subprocess
import re
import csv

def run_test(test_type, r, n, o):
    results = []
    for i in range(13):
        output = subprocess.run(['./run', f'-r {r}', f'-n {n}', f'-o {o}', test_type], capture_output=True, text=True)
        speedup_line = re.search("Speedup: (\d+\.\d+)", output.stdout)
        if speedup_line:
            speedup = float(speedup_line.group(1))
            if i >= 3:
                results.append(speedup)
    if (len(results) == 0):
        print(['./run', f'-r {r}', f'-n {n}', f'-o {o}', test_type])
    return sum(results) / len(results)

# test_types = ['-s', '-t', '-h']
test_types = ['-s']
read_write_ratios = [0.25, 0.5, 0.75]

with open('results.csv', 'w', newline='') as csvfile:
    fieldnames = ['test_type', 'r', 'n', 'o', 'average']
    writer = csv.DictWriter(csvfile, fieldnames=fieldnames)
    writer.writeheader()

    total_tests = len(test_types) * len(read_write_ratios) * (13 * 2)
    test_count = 0

    for test_type in test_types:
        for r in read_write_ratios:
            # Test scaling thread value
            for n in range(1, 14):
                average = run_test(test_type, r, 2**n, 100)
                writer.writerow({'test_type': test_type, 'r': r, 'n': 2**n, 'o': 16, 'average': average})
                test_count += 1
                print(f'Progress: {test_count}/{total_tests} ({(test_count/total_tests)*100:.2f}%)')

            # Test scaling ops value
            for o in range(1, 14):
                average = run_test(test_type, r, 100, 2**o)
                writer.writerow({'test_type': test_type, 'r': r, 'n': 16, 'o': 2**o, 'average': average})
                test_count += 1
                print(f'Progress: {test_count}/{total_tests} ({(test_count/total_tests)*100:.2f}%)')

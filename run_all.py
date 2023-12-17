from run_file import get_points, compile_file
import sys


def get_average(alg, diff):
    print()

    sum = 0
    for i in range(1, 8):
        points = get_points(alg, diff, str(i), f'output-{alg}-all.txt')
        print(f'{diff}-{i}: {points}')
        sum += points

    return sum / 7


def main():
    alg = sys.argv[1]
    
    if sys.argv[-1] != 'nc': # nc = no compilation
        compile_file(alg)

    difficulties = ('easy', 'med', 'hard')
    
    if len(sys.argv) > 2:
        difficulties = [sys.argv[2]]

    print(f'Algorithm: {alg}')

    avg = {}
    
    for diff in difficulties:
        avg[diff] = get_average(alg, diff)

    print()
    for diff, avg in avg.items():
        print(f'{diff: >4} avg : {avg:.1f}')
    print()

if __name__ == '__main__':
    main()
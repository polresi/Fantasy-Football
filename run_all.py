from run_file import get_points, compile_file
import sys


def get_average(alg, diff):
    print()

    sum = 0
    for i in range(1, 8):
        points = get_points(alg, diff, str(i))
        print(f'{diff}-{i}: {points}')
        sum += points

    return int(sum / 7)


def main():
    alg = sys.argv[1]
    print(f'Algorithm: {alg}')
    compile_file(alg)

    avg = {}
    difficulties = ('easy', 'med', 'hard') if alg != 'exh' else ('easy', 'med')
    
    for diff in difficulties:
        avg[diff] = get_average(alg, diff)

    print()
    for diff, avg in avg.items():
        print(f'{diff: >4} avg : {avg}')
    print()

if __name__ == '__main__':
    main()
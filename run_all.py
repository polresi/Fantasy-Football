from run_file import get_points, compile_file
import sys

def main():
    alg = sys.argv[1]
    compile_file(alg)

    difficulties = ('easy', 'med', 'hard') if alg != 'exh' else ('easy', 'med')
    
    for diff in difficulties:
        avg = int( sum( get_points(alg, diff, str(i)) for i in range(1, 8) ) / 7 )
        print(f'{diff: >4} : {avg: >4}')

if __name__ == '__main__':
    main()
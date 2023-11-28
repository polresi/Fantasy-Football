import subprocess
from run_file import get_points


def compile_file(alg):
    try:
        subprocess.run(["g++", alg+'.cc', "-o", alg, "-O3", "-Wall"], check=True)
    
    except subprocess.CalledProcessError as e:
        print(f"Error compiling the file: {e}")
    

def main():
    alg = 'greedy'
    compile_file(alg)

    for diff in ('easy', 'med', 'hard'):
        avg = int( sum( get_points(alg, diff, str(i)) for i in range(1, 8) ) / 7 )
        # formatted_avg = f'{avg:,}'
        print(f'{diff: >4} : {avg: >11,}')

if __name__ == '__main__':
    main()
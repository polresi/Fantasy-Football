import subprocess
import sys

def get_points(alg, diff, num, output_file='output.txt'):
    arguments = ['data_base.txt', 'new_benchs/'+diff+'-'+num+'.txt', output_file]

    try:
        subprocess.run(['./'+alg] + arguments, check=True)
        res = subprocess.run(['./checker'] + arguments, check=True, text=True, capture_output=True)
        assert res.stdout == 'OK\n'
    
    except subprocess.CalledProcessError as e:
        print(f"Error running the executable: {e}")
    
    except AssertionError:
        print(f"Output is not valid")


    with open(output_file, 'r') as file:
        content = file.read()
        points_line = content.split('\n')[-3]
        points = int(points_line.split()[-1])

    return points


def compile_file(alg):
    try:
        subprocess.run(["g++", alg+'.cc', "-o", alg, "-O3", "-Wall"], check=True)
    
    except subprocess.CalledProcessError as e:
        print(f"Error compiling the file: {e}")
    


def main():
    try:
        alg, diff, num = sys.argv[1:4]

        if len(sys.argv) <= 4 or sys.argv[4] != 'nc': # nc = no compilation
            compile_file(alg)

        points = get_points(alg, diff, num)
        print(f'Points: {points}')
    
    except KeyboardInterrupt:
        with open('output.txt', 'r') as file:
            content = file.read()
            points_line = content.split('\n')[-3]
            points = int(points_line.split()[-1])
        print(f'\nPoints: {points}')

if __name__ == '__main__':
    main()
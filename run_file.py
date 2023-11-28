import subprocess
import sys

def get_points(alg, diff, num):
    arguments = ['data_base.txt', 'new_benchs/'+diff+'-'+num+'.txt', 'output.txt']

    try:
        subprocess.run(['./'+alg] + arguments, check=True)
        res = subprocess.run(['./checker'] + arguments, check=True, text=True, capture_output=True)
        assert res.stdout == 'OK\n'
    
    except subprocess.CalledProcessError as e:
        print(f"Error running the executable: {e}")
    
    except AssertionError:
        print(f"Output is not valid")


    with open('output.txt', 'r') as file:
        content = file.read()
        points_line = content.split('\n')[-2]
        points = int(points_line.split()[-1])

    return points


def main():
    alg, diff, num = sys.argv[1:4]
    points = get_points(alg, diff, num)
    print(f'Points: {points}')


if __name__ == '__main__':
    main()
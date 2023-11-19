import subprocess


def run_file(executable_path, arguments):
    try:
        subprocess.run(['./' + executable_path] + arguments, check=True)
        res = subprocess.run(['./checker'] + arguments, check=True, text=True, capture_output=True).stdout
        assert res == 'OK\n'
    
    except subprocess.CalledProcessError as e:
        print(f"Error running the executable: {e}")


def get_avg(diff: str) -> float:
    sum = 0.0
    for i in range(1, 8):
        input_file = f'public_benchs/{diff}-{i}.txt'
        run_file('greedy', ['data_base.txt', input_file, 'output.txt'])

        with open('output.txt', 'r') as file:
            content = file.read()
            # print(content)
            points_line = content.split('\n')[-2]
            points = int(points_line.split()[-1])

        sum += points

    return sum / 7

        
def main():
    subprocess.run(["g++", "greedy.cc", "-O3", "-o", "greedy", "-Wall"])
    for diff in ('easy', 'med', 'hard'):
        print(f'{diff} : {get_avg(diff)}')

if __name__ == '__main__':
    main()
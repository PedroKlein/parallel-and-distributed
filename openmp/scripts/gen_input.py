import random
import string
import sys

if len(sys.argv) != 2:
    print("Usage: python gen_input.py <N_ELEMENTS>")
    sys.exit(1)

SIZE_ELEMENTS = 7
N_UNIQUE = 8


N_ELEMENTS = int(sys.argv[1])
def generate_dataset(n=N_ELEMENTS, length=SIZE_ELEMENTS, filename=f'./data/in_{N_ELEMENTS}.in'):
    assert (n & (n - 1)) == 0, "n must be a power of 2"
    unique_strings = set()
    while len(unique_strings) < N_UNIQUE:
        s = ''.join(random.choices(string.ascii_letters + string.digits, k=length))
        unique_strings.add(s)
    unique_strings = list(unique_strings)

    with open(filename, 'w') as f:
        f.write(f"{n}\n")
        for i in range(n):
            s = ''.join(random.choices(string.ascii_letters + string.digits, k=length))
            # f.write(f"{unique_strings[i % N_UNIQUE]}\n")
            f.write(f"{s}\n")


generate_dataset(N_ELEMENTS)

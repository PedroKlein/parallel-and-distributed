import random
import string

N_ELEMENTS = 64
def generate_dataset(n=N_ELEMENTS, length=8, filename=f'./data/in_{N_ELEMENTS}.in'):
    assert (n & (n - 1)) == 0, "n must be a power of 2"
    unique_strings = set()
    while len(unique_strings) < 8:
        s = ''.join(random.choices(string.ascii_letters + string.digits, k=length))
        unique_strings.add(s)
    unique_strings = list(unique_strings)

    with open(filename, 'w') as f:
        f.write(f"{n}\n")
        for i in range(n):
            f.write(f"{unique_strings[i % 8]}\n")

# Generate 2^10 = 1024 strings of 8 characters each
generate_dataset(1024)

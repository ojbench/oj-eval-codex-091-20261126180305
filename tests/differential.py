"""Compare the executable with a sorted-list model of the reference driver."""
import bisect
import random
import subprocess
import sys


def check(seed, executable):
    rng = random.Random(seed)
    sets = [[]]
    current = None
    commands, expected = [], []
    for _ in range(10000):
        op = rng.randrange(7)
        if op == 2 and len(sets) == 25:
            op = 0
        a = rng.randrange(len(sets))
        b = rng.choice([-(1 << 63), (1 << 63) - 1, rng.randrange(-100, 101)])
        values = sets[a]
        index = bisect.bisect_left(values, b)
        present = index < len(values) and values[index] == b
        if op == 0:
            commands.append(f"0 {a} {b}")
            if not present:
                values.insert(index, b)
                current = (a, b)
        elif op == 1:
            commands.append(f"1 {a} {b}")
            if present:
                values.pop(index)
            if current == (a, b):
                current = None
        elif op == 2:
            commands.append(f"2 {a}")
            sets.append(values.copy())
        elif op == 3:
            commands.append(f"3 {a} {b}")
            expected.append("true" if present else "false")
            if present:
                current = (a, b)
        elif op == 4:
            c = rng.choice([-(1 << 63), (1 << 63) - 1, rng.randrange(-100, 101)])
            commands.append(f"4 {a} {b} {c}")
            expected.append(str(max(0, bisect.bisect_right(values, c) - index)))
        else:
            commands.append(str(op))
            if current is not None:
                owner, key = current
                values = sets[owner]
                index = bisect.bisect_left(values, key) + (-1 if op == 5 else 1)
                current = (owner, values[index]) if 0 <= index < len(values) else None
            expected.append(str(current[1] if current is not None else -1))
    result = subprocess.run([executable], input="\n".join(commands) + "\n",
                            text=True, capture_output=True, check=True)
    assert result.stdout.splitlines() == expected, f"Mismatch at seed {seed}"


if __name__ == "__main__":
    executable = sys.argv[1] if len(sys.argv) > 1 else "./code"
    for seed in range(50):
        check(seed, executable)
    print("Passed 50 seeds, 500,000 operations")

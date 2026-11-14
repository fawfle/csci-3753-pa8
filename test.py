import subprocess
import re
import statistics

NUM_RUNS = 200
ratios = []

for i in range(NUM_RUNS):
    result = subprocess.run(
        ["./test-predict"],
        capture_output=True,
        text=True
    )

    output = result.stdout + result.stderr

    match = re.search(
        r"ratio blocked/compute\s*=\s*([\d.eE+-]+)",
        output
    )

    if match:
        ratio = float(match.group(1))
        ratios.append(ratio)
        print(f"Run {i + 1}: {ratio:.8f}")
    else:
        print(f"Run {i + 1} failed:")
        print(output)

if ratios:
    print(f"\nSuccessful runs: {len(ratios)}/{NUM_RUNS}")
    print(f"Average ratio: {statistics.mean(ratios):.8f}")
    print(f"Minimum ratio: {min(ratios):.8f}")
    print(f"Maximum ratio: {max(ratios):.8f}")

    if len(ratios) > 1:
        print(f"Standard deviation: {statistics.stdev(ratios):.8f}")

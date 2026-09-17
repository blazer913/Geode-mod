from dataclasses import dataclass

@dataclass
class Event:
    frame: int

def width(results, i, limit=10):
    if not results[i]:
        return 0
    l = r = 0
    for d in range(1, limit + 1):
        if results.get((i, -d), False): l = -d
        else: break
    for d in range(1, limit + 1):
        if results.get((i, d), False): r = d
        else: break
    return r - l + 1

if __name__ == "__main__":
    # Example: valid shifts -3..+2 => 6-frame window.
    results = {0: True}
    for d in (-3,-2,-1,1,2):
        results[(0,d)] = True
    assert width(results, 0) == 6
    print("window math OK")

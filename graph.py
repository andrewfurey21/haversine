import matplotlib.pyplot as plt
import sys

def graph_perf_data(filename:str):
    with open(filename, "r") as f:
        str_data = f.read().strip().split("\n")
        data = [float(s) for s in str_data]
        last = 0
        total_pf = 0
        for i, num in enumerate(data):
            if num != 0:
                print(i, num, i - last)
                total_pf += num
                last = i
        print("Number of page faults: ", total_pf)
        plt.plot(data)
        plt.show()


if __name__ == "__main__":
    assert len(sys.argv) == 2, "Usage: graph.py <name>"
    name = sys.argv[1]
    graph_perf_data(name)



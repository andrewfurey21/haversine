import matplotlib.pyplot as plt

def graph_perf_data(filename:str):
    with open(filename, "r") as f:
        str_data = f.read().strip().split("\n")
        data = [float(s) for s in str_data]
        print(min(data))
        print(max(data))
        plt.plot(data)
        plt.show()


if __name__ == "__main__":
    # graph_perf_data("fread_clocks")
    graph_perf_data("fread_page_faults")

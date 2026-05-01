import matplotlib.pyplot as plt

filename = "fstream.read"
with open(filename, "r") as f:
    str_data = f.read().strip().split("\n")
    data = [float(s) for s in str_data]
    plt.plot(data)
    plt.show()

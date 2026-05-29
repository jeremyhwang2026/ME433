import csv
import matplotlib.pyplot as plt
import numpy as np

t = [] # column 0
data1 = [] # column 1
A = 0.99
B = 1.00 - A
fc = 10.0 
b = 200.0
y_filtered = []

with open('sigD.csv') as f:
    # open the csv file
    reader = csv.reader(f)
    for row in reader:
        # read the rows 1 one by one
        t.append(float(row[0])) # leftmost column
        data1.append(float(row[1])) # second column

'''
for i in range(min(20, len(t))):
    # print the data to verify it was read
    print(str(t[i]) + ", " + str(data1[i]))
'''

num_points = len(t)
total_time = t[-1]
sample_rate = num_points / total_time
# print(f"sample rate: {sample_rate:.2f} samples per second")


Fs = sample_rate # sample rate
y = data1 # the data to make the fft from
n = len(y) # length of the signal


###

fc_norm = fc / Fs
b_norm = b / Fs

X = int(4 / b_norm)
if X % 2 == 0:
    X += 1

h = []
M = X - 1
for i in range(X):
    n_idx = i - M / 2
    if n_idx == 0:
        h_val = 2 * fc_norm
    else:
        h_val = np.sin(2 * np.pi * fc_norm * n_idx) / (np.pi * n_idx)
    h.append(h_val)

weights = []
for i in range(X):
    w = 0.42 - 0.5 * np.cos(2 * np.pi * i / M) + 0.08 * np.cos(4 * np.pi * i / M)
    weights.append(h[i] * w)

weight_sum = sum(weights)
weights = [w / weight_sum for w in weights]


for i in range(n):
    output_val = 0.0
    for k in range(X):
        index = i - k
        if index >= 0:
            output_val += weights[k] * y[index]
        else:
            output_val += weights[k] * 0.0
    y_filtered.append(output_val)



k = np.arange(n)
T = n/Fs
frq = k/T # two sides frequency range
frq = frq[range(int(n/2))] # one side frequency range

Y_orig = np.fft.fft(y)/n
Y_orig = Y_orig[range(int(n/2))]

Y_filt = np.fft.fft(y_filtered)/n
Y_filt = Y_filt[range(int(n/2))]


fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(8, 6))

# Time domain plot (Original vs Filtered)
ax1.plot(t, y, color='k', label='Original')
ax1.plot(t, y_filtered, color='r', label='FIR Sinc Filtered')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('Amplitude')
ax1.set_title(f'Time Domain Comparison (A={A:.3f}, B={B:.3f}, Filter=Low-pass Sinc - Blackman)')
ax2.legend()

# Frequency domain plot (Original vs Filtered FFT)
ax2.loglog(frq, abs(Y_orig), color='k', label='Original FFT') 
ax2.loglog(frq, abs(Y_filt), color='r', label='FIR Filtered FFT')
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
ax2.set_title(f'Frequency Domain Performance (Cutoff Frequency={fc}Hz, Bandwidth={b}Hz)')
ax2.legend()

plt.tight_layout()
plt.show()
import csv
import matplotlib.pyplot as plt
import numpy as np

t = [] # column 0
data1 = [] # column 1
A = 0.9
B = 1.00 - A
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


for i in range(n):
    if i == 0:
        prev_avg = 0.0
    else:
        prev_avg = y_filtered[i-1]
        
    current_avg = A * prev_avg + B * y[i]
    y_filtered.append(current_avg)


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
ax1.plot(t, y, 'k', label='Original')
ax1.plot(t, y_filtered, 'r', label='Filtered')
ax1.set_xlabel('Time (s)')
ax1.set_ylabel('Amplitude')
ax1.set_title(f'Time Domain Signal (A={A:.3f} and B={B:.3f})')
ax1.legend()

# Frequency domain plot (Original vs Filtered FFT)
ax2.loglog(frq, abs(Y_orig), 'k', label='Original FFT') 
ax2.loglog(frq, abs(Y_filt), 'r', label='Filtered FFT')
ax2.set_xlabel('Freq (Hz)')
ax2.set_ylabel('|Y(freq)|')
ax2.set_title(f'Frequency Domain (FFT) (A={A:.3f} and B={B:.3f})')
ax2.legend()
plt.tight_layout()
plt.show()
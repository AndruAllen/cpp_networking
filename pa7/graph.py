import matplotlib.pyplot as plt
#import numpy as np
#import pandas as pd
import pprint as pp

# outputs
f = open('output.txt', 'r')
# colN = [] # same for out purposes
colW = [None]
colB = []

temp = []
for line in f:
  temp_row = line.strip().split(',')
  # colN.append(int(temp_row[0]))
  if(int(temp_row[1]) != colW[-1]):
    colW.append(int(temp_row[1]))
    colB.append(temp)
    temp = []
  temp.append(int(temp_row[2]))
if(int(temp_row[1]) != colW[-1]):
  colW.append(int(temp_row[1]))
  colB.append(temp)
else:
  colB.append(temp)
colW = colW[1:]
colB = colB[1:]

#print(len(colW),colW)
#print(len(colB),colB)

# times
f = open('times2.txt', 'r')

numIters = len(colB[0])
secs = []
time_sections = []
cnt = 0;
for line in f:
  temp_row = line.strip().split(',')
  cnt += 1;
  # [sec = 1, musec = 100089]
  secs.append(float(temp_row[0][7:]+'.'+temp_row[1][9:-1]))
  if(cnt == numIters):
    time_sections.append(secs)
    cnt = 0;
    secs = []


#print(len(time_sections))
#pp.pprint(time_sections)


fig = plt.figure()
ax = fig.add_axes([0.1, 0.1, 0.6, 0.75])
ax.set_ylabel('Time (s)')
ax.set_xlabel('Size of Buffer')
ax.set_yscale('log')
ax.set_xscale('log')

for i in range(len(time_sections)):
    ax.plot(colB[i] ,time_sections[i], marker='o', label=(str(colW[i]) + " worker threads"))
    ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left', borderaxespad=0.)

plt.title('For n = 10000 varying numbers of workers and size of buffer')

plt.show()

#pp.pprint(colB);

tempf1 = []
tempf2 = []
for i in range(len(colB[0])):
  temp1 = []
  temp2 = []
  for j in range(len(colB)):
    temp1.append(colB[j][i])
    temp2.append(time_sections[j][i])
  tempf1.append(temp1)
  tempf2.append(temp2)

#pp.pprint(time_sections);
colB = tempf1;
time_sections = tempf2;
#pp.pprint(time_sections);

fig = plt.figure()
ax = fig.add_axes([0.1, 0.1, 0.6, 0.75])
ax.set_ylabel('Time (s)')
ax.set_xlabel('Number of Worker Threads')
ax.set_yscale('log')
ax.set_xscale('log')

for i in range(len(time_sections)):
    ax.plot(colW, time_sections[i], marker='o', label=(str(colB[i][0]) + " == buffer_size"))
    ax.legend(bbox_to_anchor=(1.05, 1), loc='upper left', borderaxespad=0.)

plt.title('For n = 10000 varying numbers of workers and size of buffer')

plt.show()

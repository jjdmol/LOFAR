import pickle
import matplotlib.pyplot as plt

from CsvData import csvData

print "Hello World"


pkl_file = open('beamformerStats.pkl', 'rb')

csvData = pickle.load(pkl_file)



pkl_file.close()

tabRange = range(16, 240, 8 )
channelsToTest = [1, 16, 64, 256]




resultLines = []
for channel in channelsToTest:
  line = [] 
  for tab in tabRange:
    line.append(csvData[(tab,channel)].getNamedMetric("beamFormer","Avg"))
  resultLines.append(line)



for idx in range(len(channelsToTest)):
  plt.plot(tabRange, resultLines[idx])

#plt.ylim((0,0.0035))
plt.xlim((4,max(tabRange)))

plt.legend(['1 channel', '16 channel', '64 channel', '256 channel'], loc='upper left')
plt.ylabel('Duration (sec)')
plt.xlabel('Number of tabs')
plt.show()


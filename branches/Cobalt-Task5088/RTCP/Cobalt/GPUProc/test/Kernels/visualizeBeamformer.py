import pickle
import sys
import matplotlib.pyplot as plt

from CsvData import csvData


if __name__ == "__main__":

  namedValue = sys.argv[1]
  pkl_file = open('testStats.pkl', 'rb')
  csvData = pickle.load(pkl_file)
  pkl_file.close()

  pkl_file_header = open('testStatsHeader.pkl', 'rb')
  header = pickle.load(pkl_file_header)
  pkl_file_header.close()

  resultLines = []
  for var1 in header[2]:
    line = [] 
    for var2 in header[4]:
      try:
        line.append(csvData[(var1,var2)].getNamedMetric(namedValue,"Avg"))
      except:
        line.append(0)
    resultLines.append(line)



  for idx in range(len(header[2])):
    plt.plot(header[4], resultLines[idx])

  ##plt.ylim((0,0.0035))
  #plt.xlim((4,max(tabRange)))

  #plt.legend(['1 channel', '16 channel', '64 channel', '256 channel'], loc='upper left')
  plt.ylabel('Duration (sec)')
  #plt.xlabel('Number of tabs')
  plt.show()


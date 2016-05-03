class csvData:
  """
  Contains parsed data as retrieved from nvprof
  Units are normalized normalized ( to whole seconds)
  Remove spurious quotes. 
  """
  def __init__(self, CSVLines, normalizeValues=True):
    # First line contains the name of the entries
    self.entrieNames = [ x.strip('"') for x in CSVLines[0].split(",")]

    # second entry the szie
    self.entrieUnit = CSVLines[1].split(",")

    self.entrieLines = {}
    for line in CSVLines[2:]:
      # skip empty lines
      if len(line) == 0:
        continue

      # try parsing as values if fails use string
      values = []
      for idx, entrie in enumerate(line.split(",")):
        try:
          typedValue = float(entrie)
          if normalizeValues:
            # Now do the normalizing
            if self.entrieUnit[idx] == "ns":
              typedValue *= 1e-9
            elif self.entrieUnit[idx] == "us":
              typedValue *= 1e-6
            elif self.entrieUnit[idx] == "ms":
              typedValue *= 1e-3
            elif self.entrieUnit[idx] == "s":
              pass
            elif self.entrieUnit[idx] == "%":
              pass
            else:
              raise ValueError("Could not parset the unit type for entry:" + 
                              str(entry) + " with type: " + self.entrieUnit[idx])

          values.append(typedValue)
        except:
          name = entrie.strip('"')
          # also insert the name in the values collumn
          values.append(name)
      # save the now typed line
      self.entrieLines[name] = values 

  # helper functions to allow pickeling of data
  def __getstate__(self):
    odict = {}
    odict["entrieNames"] = self.entrieNames
    odict["entrieUnit"] = self.entrieUnit
    odict["entrieLines"] = self.entrieLines

    return odict

  def __setstate__(self, dict):
    self.entrieNames = dict["entrieNames"]
    self.entrieUnit =  dict["entrieUnit"]
    self.entrieLines = dict["entrieLines"] 
 

  def getNamedMetric(self, nameLine, metric):
    """
    returns value of the metric at the entry nameline 
    or exception if either is not found
    """
    indexInList =  None
    if nameLine in self.entrieLines:
      try:
        indexInList = self.entrieNames.index(metric)
      except ValueError:
        raise ValueError("Could not find metric > " + str(metric) + " < in nvprof output. ")

    else:
      raise ValueError("Could not find >" + str(nameLine) + " < as an entrie returned by nvprof")

    # return the  value and the unit
    return self.entrieLines[nameLine][indexInList]

  def __str__(self):
    return str(self.entrieNames) + "\n" + str(self.entrieUnit) + "\n" + str(self.entrieLines)
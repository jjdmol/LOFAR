class MyATerm :
  def __init__(self) :
    print "\033[94mPython MyATerm constructor\033[0m"
    
  def evaluate(self, t, dirmap, idStation, freq, reference, normalize):
    print "\033[94m", t, dirmap, idStation, freq, reference, normalize, "\033[0m"
    
from pylofarft import _ROVisibilityIterator
import pyrap.tables

class ROVisibilityIterator(_ROVisibilityIterator) :
  
  def __init__(self, vi) :
    _ROVisibilityIterator.__init__(self, vi)
    
  @property
  def ms(self) : 
    return pyrap.tables.table(_ROVisibilityIterator.ms(self), _oper=3)
  
  

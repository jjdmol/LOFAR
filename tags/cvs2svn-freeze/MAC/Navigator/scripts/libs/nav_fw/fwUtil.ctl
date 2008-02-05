// file: fwUtil.ctl
//
// contents: miscellaneous utility functions.
//
// author: Ph. Gras CERN/EP - University of Karlsruhe
//
//  date           author      version       modification
// May 2, 01      Ph. Gras       1        creation: functions dynInsertOneItem
//                                        and dynRemoveEx, which were moved
//                                        from fwTree.ctl library. 
//----------------------------------------------------------------------------/

/** @name LIBRARY: fwUtil.ctl 
*/
//@{
/** Inserts one item in a dynamic array.
 * <p>Usage: JCOP framework internal, public
 */
fwUtil_dynInsertOneItem(dyn_anytype& dyn, anytype item, int index){
  // I have'nt managed to use dynInsertAt()
  // (does'nt work when item is itself a dyn)
	int i;
	int newSize = dynlen(dyn) + 1;
	for(i=newSize; i >index; i--)
		dyn[i] = dyn[i - 1];

	dyn[index]  = item;
}


/** Remove one item from a dynamic array.
 * <p> Usage: JCOP framework internal, public
 */
int fwUtil_dynRemoveEx(dyn_anytype& dyn, int index){
  return dynRemove(dyn, index);
}

//@}

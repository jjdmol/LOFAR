// WH_RingSimul.h: interface for the WH_RingSimul class.
//
//////////////////////////////////////////////////////////////////////

#ifndef AFX_WH_RingSimul_H__
#define AFX_WH_RingSimul_H__

#include "WorkHolder.h"
#include "DH_Test.h"
#include "DH_Ring.h"

template <class T> 
class WH_RingSimul:public WorkHolder
{
 public:
  WH_RingSimul (int channels);
  virtual ~WH_RingSimul ();
  void     process ();
  void     dump () const;

  /// Retrieve a pointer to the input data holder for the given channel
  DH_Ring<T>* getInHolder (int channel); 

  /// Retrieve a pointer to the output data holder for the given channel
  DH_Ring<T>* getOutHolder (int channel); 

 private:
  
   /** vector with pointers to the input dataholders
	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<T>*> itsInDataHolders; 

  /** vector with pointers to the output dataholders
  	  The derived classes should add a similar typed vector for their DataHolders
  */
  vector<DH_Ring<T>*> itsOutDataHolders; 

};

template <class T>
inline DH_Ring<T>* WH_RingSimul<T>::getInHolder (int channel)  { 
  return itsInDataHolders[channel]; 
}

template <class T>
inline DH_Ring<T>* WH_RingSimul<T>::getOutHolder (int channel) { 
  return itsOutDataHolders[channel];
}
/****************************************************************************/


template <class T>
inline WH_RingSimul<T>::WH_RingSimul (int channels):
WorkHolder (channels,channels)
{
  itsInDataHolders.reserve(channels);
  itsOutDataHolders.reserve(channels);
  
  for (int ch = 0; ch < getInputs(); ch++)    {
    DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
    itsInDataHolders.push_back(aDH);
  }
  
  for (int ch = 0; ch < getOutputs(); ch++)    {
    DH_Ring<DH_Test>* aDH = new DH_Ring<DH_Test>();
    itsOutDataHolders.push_back(aDH);
  }
}


template <class T>
inline WH_RingSimul<T>::~WH_RingSimul ()
{ 
}

template <class T>
inline void WH_RingSimul<T>::process ()
{
}

template <class T>
inline void WH_RingSimul<T>::dump () const
{
  cout << "WH_RingSimul " << endl;
  for (int i=0; i<getOutputs(); i++) {
    cout << " " <<  itsOutDataHolders[i]->getBuffer()[0] ;
  }
  cout << endl;
}

#endif 






// Copyright notice

#if !defined(UVPDATACACHEELEMENT_H)
#define UVPDATACACHEELEMENT_H

// $Id$

#include <uvplot/UVPDataAtomHeader.h>
#include <uvplot/UVPDataAtom.h>


//! Base class of the familie of classes that make up the data cache tree.
/*!
  Derived classes MUST override the abstract methods:
  \- size()
  \- resize()
  \- addSpectrum()
  \- getSpectrum()
  \- getLocationOfSpectrum()
 */

class UVPDataCacheElement
{
 public:

  //! Constructor
  UVPDataCacheElement();

  //! \returns the number of elements that this node contains.
  virtual unsigned int size() const = 0;

  //! Sets the number of elements that this node manages.
  /*! When extending or shrinking the current array, it keeps the old
    data as much as possible. When shrinking it discards data from the
    end of the array, when growing it keeps all present data.

    \param newSize The new number of elements.
    \returns the previous number of elements.
  */
  virtual unsigned int resize(unsigned int newSize) = 0;


  //! Adds a UVPDataAtom object to itself or one of its children.
  /*!
    It should call setDirty(true) if the spectrum was added successfully.
    \returns true on success, false on failure.
   */
  virtual bool addSpectrum(const UVPDataAtomHeader &header,
                           const UVPDataAtom       &data) = 0;

  //! Retrieve the spectrum indexed by header.
  /*!  If a class does not contain the spectra themselves, it must call
    the getSpectrum method of the child that does contain it.
    \returns 0 if there is no spectrum that matches the header.
   */
  virtual const UVPDataAtom *getSpectrum(const UVPDataAtomHeader &header) const = 0;


  //!  \returns a pointer to the UVPDataCacheElement that owns the
  //! UVPDataAtom object described by the header.
  /*!
    returns 0 if there is no spectrum that matches the header.
   */
  virtual UVPDataCacheElement *getLocationOfSpectrum(const UVPDataAtomHeader &header) = 0;
  


  //! \returns true if the datastructure is modified since the last
  //! call of setDirty(false)
  bool isDirty() const;

  //! Sets or unsets the itsDirty flag.
  /*!  \param dirty must be set to false whenever a user (in this case
    another class) has accessed the data. If the datastructure is
    modified in some way, the flag should be set to true.
   */ 
  void setDirty(bool dirty = true);
  

 protected:
 private:

  bool   itsDirty;
};


#endif //UVPDATACACHEELEMENT_H

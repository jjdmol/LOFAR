// Copyright notice should go here

// $ID$

#if !defined(UVPUVCOVERAGEAREA_H)
#define UVPUVCOVERAGEAREA_H



#include <UVPDisplayArea.h>
#include <UVPImageCube.h>



class UVPUVCoverageArea : public UVPDisplayArea
{
 public:

  UVPUVCoverageArea(QWidget*            parent,
                    const UVPImageCube* data =0);

  void setData(const UVPImageCube* data=0);

  virtual void drawView();
  
 protected:
 private:

  // Just a pointer. The datastructure is managed by an external object
  const UVPImageCube* itsCurrentImage;

};


#endif // UVPUVCOVERAGEAREA_H

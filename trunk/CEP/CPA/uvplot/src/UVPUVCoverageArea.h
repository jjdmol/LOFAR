// Copyright notice should go here

#if !defined(UVPUVCOVERAGEAREA_H)
#define UVPUVCOVERAGEAREA_H

// $Id$



#include <UVPDisplayArea.h>
#include <UVPImageCube.h>

#include <qimage.h>


class UVPUVCoverageArea : public UVPDisplayArea
{
  Q_OBJECT

 public:

  UVPUVCoverageArea(QWidget*            parent,
                    const UVPImageCube* data =0);
  ~UVPUVCoverageArea();

  void setData(const UVPImageCube* data=0);

  virtual void drawView();

  public slots:
    
    void slot_paletteChanged();
    
 protected:
 private:

  // Just a pointer. The datastructure is managed by an external object
  const UVPImageCube* itsCurrentImage;
};


#endif // UVPUVCOVERAGEAREA_H

//# awimager.cc: Program to create and/or clean a LOFAR image
//# Copyright (C) 2011
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This program is free software; you can redistribute it and/or modify it
//# under the terms of the GNU General Public License as published by the Free
//# Software Foundation; either version 2 of the License, or (at your option)
//# any later version.
//#
//# This program is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
//# more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with this program; if not, write to the Free Software Foundation, Inc.,
//# 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#
//# $Id$

//# Includes
#include <lofar_config.h>
#include <LofarFT/LofarImager.h>
#include <Common/InputParSet.h>

#include <images/Images/PagedImage.h>
#include <images/Images/HDF5Image.h>
#include <images/Images/ImageFITSConverter.h>
#include <casa/Arrays/ArrayUtil.h>
#include <casa/Arrays/ArrayMath.h>
#include <casa/Arrays/ArrayIter.h>
#include <casa/Utilities/Regex.h>
#include <casa/Utilities/Assert.h>
#include <casa/OS/Directory.h>
#include <casa/OS/File.h>
#include <casa/Exceptions/Error.h>
#include <casa/OS/Timer.h>
#include <casa/OS/PrecTimer.h>
#include <casa/iostream.h>
#include <casa/sstream.h>

using namespace casa;

IPosition handlePos (const IPosition& pos, const IPosition& def)
{
  if (pos.nelements() == 0) {
    return def;
  }
  if (pos.nelements() != 2) {
    throw AipsError("Give 0 or 2 values in maskblc and masktrc");
  }
  IPosition npos(def);
  int n = npos.nelements();
  npos[n-1] = pos[1];
  npos[n-2]= pos[0];
  return npos;
}

IPosition readIPosition (const String& in)
{
  if (in.empty()) {
    return IPosition();
  }
  Vector<String> strs = stringToVector (in);
  IPosition ipos(strs.nelements());
  for (uInt i=0; i<strs.nelements(); ++i) {
    istringstream iss(strs[i]);
    iss >> ipos[i];
  }
  return ipos;
}

Quantity readQuantity (const String& in)
{
  Quantity res;
  if (!Quantity::read(res, in)) {
    throw AipsError (in + " is an illegal quantity");
  }
  return res;
}

MDirection readDirection (const String& in)
{
  Vector<String> vals = stringToVector(in);
  if (vals.size() > 3) {
    throw AipsError ("MDirection value " + in + " is invalid;"
		     " up to 3 values can be given");
  }
  MDirection::Types tp;
  if (! MDirection::getType (tp, vals[0])) {
    throw AipsError(vals[0] + " is an invalid MDirection type");
  }
  Quantity v0(0, "deg");
  Quantity v1(90, "deg");     // same default as in measures.g
  if (vals.size() > 1  &&  !vals[1].empty()) {
    v0 = readQuantity(vals[1]);
  }
  if (vals.size() > 2  &&  !vals[2].empty()) {
    v1 = readQuantity(vals[2]);
  }
  return MDirection(v0, v1, tp);
}

void readFilter (const String& filter,
                 Quantity& bmajor, Quantity& bminor, Quantity& bpa)
{
  if (filter.empty()) {
    return;
  }
  Vector<String> strs = stringToVector(filter);
  if (strs.size() != 3) {
    throw AipsError("Specify gaussian tapering filter as bmajor,bminor,bpa");
  }
  if (! strs[0].empty()) {
    bmajor = readQuantity (strs[0]);
  }
  if (! strs[1].empty()) {
    bminor = readQuantity (strs[1]);
  }
  if (! strs[2].empty()) {
    bpa = readQuantity (strs[2]);
  }
}

Matrix<Bool> readMueller (const String& str, String stokes, Bool grid)
{
  Matrix<Bool> mat(4,4, True);
  String s(str);
  s.upcase();
  if (s == "FULL") {
    s = "ALL";
  }
  if (s == "DIAGONAL") {
    mat = False;
    mat.diagonal() = True;
  } else if (s != "ALL" ) {
    mat(0,4) = mat(4,0) = False;
    if (s == "BAND1") {
      mat(0,3) = mat(1,4) = mat(3,0) = mat(4,1) = False;
    } else if (s != "BAND2") {
      throw AipsError (str + " is an invalid Mueller specification");
    }
  }
  if((stokes=="I")&&(grid)){
    for(uInt i=0;i<4;++i){
      mat(1,i)=0;
      mat(2,i)=0;
    };
  }
  if((stokes=="I")&&(!grid)){
    for(uInt i=0;i<4;++i){
      mat(1,i)=0;
      mat(2,i)=0;
      // mat(i,1)=0;
      // mat(i,2)=0;
    };
  };
  return mat;
}

void makeEmpty (Imager& imager, const String& imgName, Int fieldid)
{
  CoordinateSystem coords;
  AlwaysAssert (imager.imagecoordinates(coords), AipsError);
  String name(imgName);
  imager.makeEmptyImage(coords, name, fieldid);
  imager.unlock();
}

void applyFactors (PagedImage<Float>& image, const Array<Float>& factors)
{
  Array<Float> data;
  image.get (data);
  ///  cout << "apply factor to " << data.data()[0] << ' ' << factors.data()[0]<<endl;
  // Loop over channels
  for (ArrayIterator<Float> iter1(data, 3); !iter1.pastEnd(); iter1.next()) {
    // Loop over Stokes.
    ArrayIterator<Float> iter2(iter1.array(), 2);
    while (! iter2.pastEnd()) {
      iter2.array() *= factors;
      iter2.next();
    }
  }
  image.put (data);
  ///  cout << "applied factor to " << data.data()[0] << ' ' << factors.data()[0]<<endl;
}

void correctImages (const String& restoName, const String& modelName,
                    const String& residName, const String& imgName,
                    LOFAR::LofarImager&,
                    Bool CorrectElement, Bool CorrectWPlanes, Bool doRestored)
{
  // Copy the images to .corr ones.
  {
    Directory modelIn(modelName);
    modelIn.copy (modelName+".corr");
    Directory residualIn(residName);
    residualIn.copy (residName+".corr");
  }
  // Open the images.
  PagedImage<Float> modelImage(modelName+".corr");
  PagedImage<Float> residualImage(residName+".corr");
  // AlwaysAssert (residualImage.shape() == modelImage.shape()  &&
  //              restoredImage.shape() == modelImage.shape(), SynthesisError);

  // Get average primary beam and spheroidal.
  Matrix<Float> avgPB = LOFAR::LofarConvolutionFunction::getAveragePB(imgName+"0");
  Matrix<Float> spheroidCut = LOFAR::LofarConvolutionFunction::getSpheroidCut(imgName+"0");

  // String nameii("Spheroid_cut_im_element.img");
  // ostringstream nameiii(nameii);
  // PagedImage<Float> tmpi(nameiii.str().c_str());
  // Slicer slicei(IPosition(4,0,0,0,0), tmpi.shape(), IPosition(4,1,1,1,1));
  // Array<Float> spheroidCutElement;
  // tmpi.doGetSlice(spheroidCutElement, slicei);

  // Use the inner part of the beam and spheroidal.
  Int nximg = residualImage.shape()[0];
  Int nxpb  = avgPB.shape()[0];
  Int nxsph = spheroidCut.shape()[0];
  //AlwaysAssert (restoredImage.shape()[1] == nximg  &&
  //              avgPB.shape()[1] == nxpb  &&
  //              spheroidCut.shape()[1] == nxsph  &&
  //              nxsph >= nximg  &&  nxpb >= nximg, SynthesisError);
  // Get inner parts of beam and spheroid.
  Int offpb  = (nxpb  - nximg) / 2;
  Int offsph = (nxsph - nximg) / 2;
  Array<Float> pbinner  = avgPB(Slicer(IPosition(2, offpb, offpb),
                                       IPosition(2, nximg, nximg)));
  Array<Float> sphinner = spheroidCut(Slicer(IPosition(2, offsph, offsph),
                                             IPosition(2, nximg, nximg)));
  Array<Float> ones(IPosition(sphinner.shape()),1.);
  Array<Float> factors;
  // Array<Float> sphinner_el = (spheroidCutElement(Slicer(IPosition(4, offsph, offsph,0,0),
  // 							  IPosition(4, nximg, nximg,1,1)))).nonDegenerate();

  // if(CorrectElement){
  //   factors = sphinner_el *sphinner / sqrt(pbinner);//sphinner_el * sphinner / sqrt(pbinner);
  // } else{
  //   factors = ones / sqrt(pbinner);//sphinner_el * sphinner / sqrt(pbinner);
  // }
  //if(CorrectWPlanes){factors *= sphinner;}

  factors = ones / sqrt(pbinner);
  cout<<"Final normalisation"<<endl;
  applyFactors (modelImage, factors);
  applyFactors (residualImage, factors);
  if(doRestored){
    cout<<"... restored image too ..."<<endl;
    Directory restoredIn(restoName);
    restoredIn.copy (restoName+".corr");
    PagedImage<Float> restoredImage(restoName+".corr");
    applyFactors (restoredImage, factors);
  }
}


int main (Int argc, char** argv)
{
  try {
    LOFAR::InputParSet inputs;
    // define the input structure
    inputs.setVersion("2011Oct05-CT/SvdT/JvZ/GvD");
    inputs.create ("ms", "",
		   "Name of input MeasurementSet",
		   "string");
    inputs.create ("image", "",
		   "Name of output image file (default is <msname-stokes-mode-nchan>.img)",
		   "string");
    inputs.create ("fits", "no",
		   "Name of output image fits file ('no' means no fits file) empty is <imagename>.fits",
		   "string");
    inputs.create ("hdf5", "no",
    		   "Name of output image HDF5 file ('no' means no HDF5 file) empty is <imagename>.hdf5",
    		   "string");
    inputs.create ("prior", "",
		   "Name of prior image file (default is <imagename>.prior",
		   "string");
    inputs.create ("model", "",
		   "Name of model image file (default is <imagename>.model",
		   "string");
    inputs.create ("restored", "",
		   "Name of restored image file (default is <imagename>.restored",
		   "string");
    inputs.create ("residual", "",
		   "Name of residual image file (default is <imagename>.residual",
		   "string");
    inputs.create ("psf", "",
		   "Name of psf image file (default is <imagename>.psf",
		   "string");
    inputs.create ("data", "DATA",
		   "Name of DATA column to use",
		   "string");
    inputs.create ("mode", "mfs",
		   "Imaging mode (mfs, channel, or velocity)",
		   "string");
    inputs.create ("filter", "",
                   "Apply gaussian tapering filter; specify as major,minor,pa",
                   "string");
    inputs.create ("nscales", "5",
                   "Number of scales for MultiScale Clean",
                   "int");
    inputs.create ("weight", "briggs",
		   "Weighting scheme (uniform, superuniform, natural, briggs (robust), briggsabs, or radial",
		   "string");
    inputs.create ("noise", "1.0",
		   "Noise (in Jy) for briggsabs weighting",
		   "float");
    inputs.create ("robust", "0.0",
		   "Robust parameter",
		   "float");
    inputs.create ("wprojplanes", "0",
		   "if >0 specifies nr of convolution functions to use in W-projection",
		   "int");
    inputs.create ("padding", "1.0",
		   "padding factor in image plane (>=1.0)",
		   "float");
    inputs.create ("timewindow", "300.0",
                   "width of time window (in sec) where AW-term is constant",
                   "double");
    inputs.create ("wmax", "10000.0",
		   "omit data with w-term > wmax (in meters)",
		   "float");
    inputs.create ("muellergrid", "all",
		   "Mueller elements to use when gridding (all,diagonal,band1,band2)",
		   "string");
    inputs.create ("muellerdegrid", "all",
		   "Mueller elements to use when degridding (all,diagonal,band1,band2)",
		   "string");
    inputs.create ("cachesize", "512",
		   "maximum size of gridding cache (in MBytes)",
		   "int");
    inputs.create ("displayprogress", "false",
		   "show the progress of the imaging process?",
		   "bool");
    inputs.create ("stokes", "IQUV",
		   "Stokes parameters to image (e.g. IQUV)",
		   "string");
    inputs.create ("nfacets", "1",
                   "number of facets in x or y",
                   "int");
    inputs.create ("npix", "256",
		   "number of image pixels in x and y direction",
		   "int");
    inputs.create ("cellsize", "1arcsec",
		   "pixel width in x and y direction",
		   "quantity string");
    inputs.create ("phasecenter", "",
		   "phase center to be used (e.g. 'j2000, 05h30m, -30.2deg')",
		   "direction string");
    inputs.create ("field", "0",
		   "field id to be used",
		   "int");
    inputs.create ("spwid", "0",
		   "spectral window id(s) to be used",
		   "int vector");
    inputs.create ("chanmode", "channel",
		   "frequency channel mode",
		   "string");
    inputs.create ("nchan", "1",
		   "number of frequency channels to select from each spectral window (one number per spw)",
		   "int vector");
    inputs.create ("chanstart", "0",
		   "first frequency channel per each spw (0-relative)",
		   "int vector");
    inputs.create ("chanstep", "1",
		   "frequency channel step per each spw",
		   "int vector");
    inputs.create ("img_nchan", "1",
		   "number of frequency channels in image",
		   "int");
    inputs.create ("img_chanstart", "0",
		   "first frequency channel in image (0-relative)",
		   "int");
    inputs.create ("img_chanstep", "1",
		   "frequency channel step in image",
		   "int");
    inputs.create ("select", "",
		   "TaQL selection string for MS",
		   "string");
    inputs.create ("operation", "image",
                   ///		   "Operation (empty,image,clark,hogbom,csclean,multiscale,entropy)",
		   "Operation (empty,image,csclean,predict,psf,mfclark)",
		   "string");
    inputs.create ("niter", "1000",
		   "Number of clean iterations",
		   "int");
    inputs.create ("gain", "0.1",
		   "Loop gain for cleaning",
		   "float");
    inputs.create ("threshold", "0Jy",
		   "Flux level at which to stop cleaning",
		   "quantity string");
    inputs.create ("targetflux", "1.0Jy",
		   "Target flux for maximum entropy",
		   "quantity string");
    inputs.create ("sigma", "0.001Jy",
		   "deviation for maximum entropy",
		   "quantity string");
    inputs.create ("fixed", "False",
		   "Keep clean model fixed",
		   "bool");
    inputs.create ("constrainflux", "False",
		   "Constrain image to match target flux? For max entropy",
		   "bool");
    inputs.create ("prefervelocity", "True",
                   "Should FITS image spectral axis be velocity or frequency",
                   "bool");
    inputs.create ("mask", "",
		   "Name of the mask to use in cleaning",
		   "string");
    inputs.create ("maskblc", "0,0",
		   "bottom-left corner of mask region",
		   "int vector");
    inputs.create ("masktrc", "image shape",
		   "top-right corner of mask region",
		   "int vector");
    inputs.create ("uservector", "0",
		   "user-defined scales for MultiScale clean",
		   "float vector");
    inputs.create ("maskvalue", "-1.0",
		   "Value to store in mask region; if given, mask is created; if mask not exists, defaults to 1.0",
		   "float");
    inputs.create ("verbose", "0",
		   "0=some output, 1=more output, 2=even more output",
		   "int");
    inputs.create ("maxsupport", "1024",
		   "maximum support size for W convolution functions",
		   "int");
    inputs.create ("oversample", "8",
		   "oversampling for convolution functions",
		   "int");
    inputs.create ("uvdist", "",
		   "UV Range",
		   "string");
    inputs.create ("RefFreq", "",
		   "Reference Frequency (Hz)",
		   "Double");
    inputs.create ("nterms", "1",
		   "Number of Taylor terms",
		   "int");
    inputs.create ("UseLIG", "false",
		   "Use gridder using linear interpolation (not working yet, never to be)",
		   "bool");
    inputs.create ("UseEJones", "true",
                   "Use the beam for the calculation of the convolution function (not working yet)",
                   "bool");
    inputs.create ("applyIonosphere", "false",
                   "apply ionospheric correction",
                   "bool");
    inputs.create ("splitbeam", "true",
                   "Evaluate station beam and element beam separately (splitbeam = true is faster)",
                   "bool");
    inputs.create ("PBCut", "1e-2",
		   "Level below which the dirty images will be set to zero. Expressed in units of peak primary beam.",
		   "Double");
    inputs.create ("cyclefactor", "1.5",
		   "Cycle Factor. See Casa definition.",
		   "Double");
    inputs.create ("cyclespeedup", "-1",
		   "Cycle Factor. See Casa definition.",
		   "Double");
    inputs.create ("ModelImPredict", "",
		   "Input Model image for the predict",
		   "string");
    inputs.create ("PsfImage", "",
		   "Input PSF image for the cleaning",
		   "string");
    inputs.create ("ApplyElement", "0",
		   "If turned to true, apply the element beam every TWElement.",
		   "int");
    inputs.create ("ApplyBeamCode", "0",
		   "Ask developers.",
		   "int");
    inputs.create ("UseMasks", "true",
		   "When the element beam is applied (StepApplyElement), the addictional step of convolving the grid can be made more efficient by computing masks. If true, it will create a directory in which it stores the masks.",
		   "bool");
    inputs.create ("UVmin", "0",
		   "Minimum UV distance (klambda)",
		   "Double");
    inputs.create ("UVmax", "1000",
		   "Maximum UV distance (klambda)",
		   "Double");
    inputs.create ("RowBlock", "0",
		   "In certain obscure circounstances (taql, selection using uvdist), the RowBlocking used by the imager calculated from the timewindow value is not correct. This parameter can be used to specify the RowBlocking.",
		   "int");
    inputs.create ("MakeDirtyCorr", "false",
		   "Image plane correction.",
		   "bool");
    inputs.create ("UseWSplit", "true",
		   "W split.",
		   "bool");
    inputs.create ("TWElement", "20.",
		   "Timewindow for applying the element beam in hours. ChunkSize otherwise.",
		   "Double");
    inputs.create ("SpheSupport", "15",
		   "Spheroidal/Aterm Support.",
		   "Double");
    inputs.create ("t0", "-1",
		   "tmin in minutes since beginning.",
		   "Double");
    inputs.create ("t1", "-1",
		   "tmax in minutes since beginning.",
		   "Double");
    inputs.create ("SingleGridMode", "true",
		   "If set to true, then the FTMachine uses only one grid.",
		   "bool");
    inputs.create ("FindNWplanes", "true",
		   "If set to true, then find the optimal number of W-planes, given spheroid support, wmax and field of view.",
		   "bool");
    inputs.create ("ChanBlockSize", "0",
		   "Channel block size. Use if you want to use a different CF per block of channels.",
		   "int");
    
    // inputs.create ("FillFactor", "1",
    // 		   "Fraction of the data that will be selected from the selected MS. (don't use it yet)",
    // 		   "Double");
 
    // Fill the input structure from the command line.
    inputs.readArguments (argc, argv);

    // Get the input specification.
    Bool fixed          = inputs.getBool("fixed");
    Bool UseLIG         = inputs.getBool("UseLIG");
    Bool UseWSplit         = inputs.getBool("UseWSplit");
    Bool UseEJones      = inputs.getBool("UseEJones");
    Bool MakeDirtyCorr  = inputs.getBool("MakeDirtyCorr");
    Bool applyIonosphere = inputs.getBool("applyIonosphere");
    Bool splitbeam = inputs.getBool("splitbeam");
    Bool constrainFlux  = inputs.getBool("constrainflux");
    Bool preferVelocity = inputs.getBool("prefervelocity");
    Bool displayProgress= inputs.getBool("displayprogress");

    Long cachesize   = inputs.getInt("cachesize");
    Int fieldid      = inputs.getInt("field");
    Vector<Int> spwid(inputs.getIntVector("spwid"));
    Int npix         = inputs.getInt("npix");
    Int nfacet       = inputs.getInt("nfacets");
    Vector<Int> nchan(inputs.getIntVector("nchan"));
    Vector<Int> chanstart(inputs.getIntVector("chanstart"));
    Vector<Int> chanstep(inputs.getIntVector("chanstep"));
    Int img_nchan    = inputs.getInt("img_nchan");
    Int img_start    = inputs.getInt("img_chanstart");
    Int img_step     = inputs.getInt("img_chanstep");
    Int wplanes      = inputs.getInt("wprojplanes");
    Int niter        = inputs.getInt("niter");
    Int nscales      = inputs.getInt("nscales");
    Int verbose      = inputs.getInt("verbose");
    Int maxsupport   = inputs.getInt("maxsupport");
    Int oversample   = inputs.getInt("oversample");
    Int StepApplyElement   = inputs.getInt("ApplyElement");
    Int ApplyBeamCode   = inputs.getInt("ApplyBeamCode");
    if ((StepApplyElement%2 == 0)&&((StepApplyElement%2 != 0))) {
      StepApplyElement++;
    }
    Int nterms   = inputs.getInt("nterms");
    Vector<Double> userScaleSizes(inputs.getDoubleVector("uservector"));
    Double padding   = inputs.getDouble("padding");
    Double gain      = inputs.getDouble("gain");
    Double maskValue = inputs.getDouble("maskvalue");
    String mode      = inputs.getString("mode");
    String operation = inputs.getString("operation");
    String weight    = inputs.getString("weight");
    double noise     = inputs.getDouble("noise");
    double robust    = inputs.getDouble("robust");
    double timewindow= inputs.getDouble("timewindow");
    double wmax      = inputs.getDouble("wmax");
    String filter    = inputs.getString("filter");
    String stokes    = inputs.getString("stokes");
    String chanmode  = inputs.getString("chanmode");
    String cellsize  = inputs.getString("cellsize");
    String phasectr  = inputs.getString("phasecenter");
    String sigmaStr  = inputs.getString("sigma");
    String targetStr = inputs.getString("targetflux");
    String threshStr = inputs.getString("threshold");
    String msName    = inputs.getString("ms");
    String imgName   = inputs.getString("image");
    String fitsName  = inputs.getString("fits");
    String hdf5Name  = inputs.getString("hdf5");
    String modelName = inputs.getString("model");
    String priorName = inputs.getString("prior");
    String restoName = inputs.getString("restored");
    String residName = inputs.getString("residual");
    String psfName   = inputs.getString("psf");
    String imageType = inputs.getString("data");
    String select    = inputs.getString("select");
    String uvdist    = inputs.getString("uvdist");
    String maskName  = inputs.getString("mask");
    String mstrBlc   = inputs.getString("maskblc");
    String mstrTrc   = inputs.getString("masktrc");
    Double RefFreq   = inputs.getDouble("RefFreq");
    Double PBCut   = inputs.getDouble("PBCut");
    Double cyclefactor   = inputs.getDouble("cyclefactor");
    Double cyclespeedup  = inputs.getDouble("cyclespeedup");
    Matrix<Bool> muelgrid   = readMueller (inputs.getString("muellergrid"), stokes, true);
    Matrix<Bool> mueldegrid = readMueller (inputs.getString("muellerdegrid"), stokes, false);
    String ModelImPredict    = inputs.getString("ModelImPredict");
    String PsfImage    = inputs.getString("PsfImage");
    Bool Use_masks    = inputs.getBool("UseMasks");
    Int RowBlock   = inputs.getInt("RowBlock");
    Double UVmin   = inputs.getDouble("UVmin");
    Double UVmax   = inputs.getDouble("UVmax");
    Double TWElement   = inputs.getDouble("TWElement");
    Double SpheSupport = inputs.getDouble("SpheSupport");
    Double t0 = inputs.getDouble("t0");
    Double t1 = inputs.getDouble("t1");
    Bool SingleGridMode    = inputs.getBool("SingleGridMode");
    Bool FindNWplanes    = inputs.getBool("FindNWplanes");
    Int ChanBlockSize   = inputs.getInt("ChanBlockSize");
    
    //Double FillFactor= 1.;//inputs.getDouble("FillFactor");

    // Check and interpret input values.
    Quantity qcellsize = readQuantity (cellsize);
    if (msName.empty()) {
      throw AipsError("An MS name must be given like ms=test.ms");
    }
    imageType.downcase();
    if (imageType == "data") {
      imageType = "observed";
    } else if (imageType == "corrected_data") {
      imageType = "corrected";
    } else if (imageType == "model_data") {
      imageType = "model";
    } else if (imageType == "residual_data") {
      imageType = "residual";
    }
    if (select.empty()) {
      select = "ANTENNA1 != ANTENNA2";
    } else {
      select = '(' + select + ") && ANTENNA1 != ANTENNA2";
    }
    if (imgName.empty()) {
      imgName = msName;
      imgName.gsub (Regex("\\..*"), "");
      imgName.gsub (Regex(".*/"), "");
      imgName += '-' + stokes + '-' + mode + String::toString(img_nchan)
	+ ".img";
    }
    if (fitsName == "no") {
      fitsName = String();
    } else if (fitsName.empty()) {
      fitsName = imgName + ".fits";
    }
    if (hdf5Name == "no") {
      hdf5Name = String();
    }
    if (priorName.empty()) {
      priorName = imgName + ".prior";
    }
    if (modelName.empty()) {
      modelName = imgName + ".model";
    }
    if (restoName.empty()) {
      restoName = imgName + ".restored";
    }
    if (residName.empty()) {
      residName = imgName + ".residual";
    }
    if (psfName.empty()) {
      psfName = imgName + ".psf";
    }
    if (weight == "robust") {
      weight = "briggs";
    } else if (weight == "robustabs") {
      weight = "briggsabs";
    }
    string rmode = "norm";
    if (weight == "briggsabs") {
      weight = "briggs";
      rmode  = "abs";
    }
    bool doShift = False;
    MDirection phaseCenter;
    if (! phasectr.empty()) {
      doShift = True;
      phaseCenter = readDirection (phasectr);
    }
    operation.downcase();
    AlwaysAssertExit (operation=="empty" || operation=="image" || operation=="csclean"|| operation=="msmfs"||operation=="predict"||operation=="psf"||operation=="mfclark");
    ///AlwaysAssertExit (operation=="empty" || operation=="image" || operation=="hogbom" || operation=="clark" || operation=="csclean" || operation=="multiscale" || operation =="entropy");
    IPosition maskBlc, maskTrc;
    Quantity threshold;
    Quantity sigma;
    Quantity targetFlux;
    Bool doClean = (operation != "empty"  &&  operation != "image"&&  operation != "psf");
    if (doClean) {
      maskBlc = readIPosition (mstrBlc);
      maskTrc = readIPosition (mstrTrc);
      threshold = readQuantity (threshStr);
      sigma = readQuantity (sigmaStr);
      targetFlux = readQuantity (targetStr);
    }
    Bool doPSF =(operation=="psf");
    if(doPSF==true){
      operation="csclean";
      niter=0;
    }
    // Get axis specification from filter.
    Quantity bmajor, bminor, bpa;
    readFilter (filter, bmajor, bminor, bpa);

    // Set the various imager variables.
    // The non-parameterized values used are the defaults in imager.g.
    MeasurementSet ms(msName, Table::Update);
    Record params;
    params.define ("timewindow", timewindow);
    params.define ("wmax", wmax);
    params.define ("mueller.grid", muelgrid);
    params.define ("mueller.degrid", mueldegrid);
    params.define ("verbose", verbose);
    params.define ("maxsupport", maxsupport);
    params.define ("oversample", oversample);
    params.define ("imagename", imgName);
    params.define ("UseLIG", UseLIG);
    params.define ("UseWSplit", UseWSplit);
    params.define ("UseEJones", UseEJones);
    params.define ("PBCut", PBCut);
    params.define ("StepApplyElement", StepApplyElement);
    params.define ("ApplyBeamCode", ApplyBeamCode);
    Bool PredictFT(false);
    if(operation=="predict"){PredictFT=true;}
    params.define ("PredictFT", PredictFT);
    params.define ("PsfImage", PsfImage);
    params.define ("UseMasksDegrid", Use_masks);
    params.define ("RowBlock", RowBlock);
    params.define ("doPSF", doPSF);
    params.define ("applyIonosphere", applyIonosphere);
    params.define ("splitbeam", splitbeam);
    params.define ("MakeDirtyCorr", MakeDirtyCorr);
    params.define ("UVmin", UVmin);
    params.define ("UVmax", UVmax);
    params.define ("TWElement", TWElement);
    params.define ("SpheSupport", SpheSupport);
    params.define ("t0", t0);
    params.define ("t1", t1);
    params.define ("SingleGridMode", SingleGridMode);
    params.define ("FindNWplanes", FindNWplanes);
    params.define ("ChanBlockSize", ChanBlockSize);

    
    //params.define ("FillFactor", FillFactor);
    
    LOFAR::LofarImager imager(ms, params);

    MSSpWindowColumns window(ms.spectralWindow());
    // ROMSObservationColumns timerange(ms.observation());
    // cout<<"timerange"<<timerange.timerange()<<endl;
    Vector<Int> wind(window.nrow());
    for(uInt iii=0;iii<window.nrow();++iii){wind(iii)=iii;};

    ROArrayColumn<Double> chfreq(window.chanFreq());

    cout<<"Number of channels: "<<chfreq(0).shape()[0]<<endl;
    if(ChanBlockSize!=0){
      AlwaysAssertExit (((chfreq(0).shape()[0]%ChanBlockSize)==0)&(ChanBlockSize<chfreq(0).shape()[0]));
    }

    Vector<Int> chansel(1);
    chansel(0)=chfreq(0).shape()[0];
    //chansel(1)=1;
    //chansel(2)=2;
    //chansel(3)=3;

    imager.setdata (chanmode,                       // mode
		    chansel,//nchan,
		    chanstart,
                    chanstep,
		    MRadialVelocity(),              // mStart
		    MRadialVelocity(),              // mStep
		    wind,//spwid,
		    Vector<Int>(1,fieldid),
		    select,                         // msSelect
                    String(),                       // timerng
                    String(),                       // fieldnames
                    Vector<Int>(),                  // antIndex
                    String(),                       // antnames
                    String(),                       // spwstring
                    uvdist,                       // uvdist
                    String(),                       // scan
                    True);                          // useModelCol


    imager.setmfcontrol(cyclefactor,          //Float cyclefactor,
  			cyclespeedup,         //Float cyclespeedup,
  			0.8,                        //Float cyclemaxpsffraction, 
  			2,                          //Int stoplargenegatives, 
  			-1,                         //Int stoppointmode,
  			"",                         //String& scaleType,
  			0.1,                        //Float minPB,
  			0.4,                        //loat constPB,
  			Vector<String>(1, ""),      //Vector<String>& fluxscale,
  			true);                      //Bool flatnoise);

    imager.defineImage (npix,                       // nx
                        npix,                       // ny
                        qcellsize,                  // cellx
                        qcellsize,                  // celly
                        stokes,                     // stokes
                        phaseCenter,                // phaseCenter
                        doShift  ?  -1 : fieldid,   // fieldid
                        mode,                       // mode
                        img_nchan,                  // nchan
                        img_start,                  // start
                        img_step,                   // step
                        MFrequency(),               // mFreqstart
                        MRadialVelocity(),          // mStart
                        Quantity(1,"km/s"),         // qstep, Def=1 km/s
			wind,//spwid,                      // spectralwindowids
                        nfacet);                    // facets

    if (operation=="predict"){
      String ftmachine("ft");
      if (wplanes > 0) {
        ftmachine = "wproject";
      }
      imager.setoptions(ftmachine,                    // ftmachine
                        cachesize*1024*(1024/8),      // cache
                        16,                           // tile
                        "SF",                         // gridfunction
                        MPosition(),                  // mLocation
                        padding,                      // padding
                        wplanes);                     // wprojplanes
      imager.ft(Vector<String>(1, ModelImPredict), "",
		False);
    } else{

      // Create empty image?
      if (operation == "empty" ) {
        makeEmpty (imager, imgName, fieldid);
      } else {

        // Define weighting.
        if (weight != "default") {
          imager.weight (weight,                      // type
                         rmode,                       // rmode
                         Quantity(noise, "Jy"),       // briggsabs noise
                         robust,                      // robust
                         Quantity(0, "rad"),          // fieldofview
                         0);                          // npixels
        }

        // If multiscale, set its parameters.
        if (operation == "multiscale") {
          String scaleMethod;
          Vector<Float> userVector(userScaleSizes.shape());
          convertArray (userVector, userScaleSizes);
          if (userScaleSizes.size() > 1) {
            scaleMethod = "uservector";
          } else {
            scaleMethod = "nscales";
          }
          imager.setscales(scaleMethod, nscales, userVector);
        }
        if (! filter.empty()) {
          imager.filter ("gaussian", bmajor, bminor, bpa);
        }
        String ftmachine("ft");
        if (wplanes > 0) {
          ftmachine = "wproject";
        }
        imager.setoptions(ftmachine,                    // ftmachine
                          cachesize*1024*(1024/8),      // cache
                          16,                           // tile
                          "SF",                         // gridfunction
                          MPosition(),                  // mLocation
                          padding,                      // padding
                          wplanes);                     // wprojplanes

        if (operation == "image") {
          Timer timer;
          PrecTimer precTimer;
          precTimer.start();
          imager.makeimage (imageType, imgName);
          precTimer.stop();
          timer.show ("makeimage");
          imager.showTimings (cout, precTimer.getReal());

          // Convert result to fits if needed.
          if (! fitsName.empty()) {
            String error;
            PagedImage<float> img(imgName);
            if (! ImageFITSConverter::ImageToFITS (error,
                                                   img,
                                                   fitsName,
                                                   64,         // memoryInMB
                                                   preferVelocity)) {
              throw AipsError(error);
            }
          }

          // Convert to HDF5 if needed.
          if (! hdf5Name.empty()) {
            PagedImage<float> pimg(imgName);
            HDF5Image<float>  himg(pimg.shape(), pimg.coordinates(), hdf5Name);
            himg.copyData (pimg);
            himg.setUnits     (pimg.units());
            himg.setImageInfo (pimg.imageInfo());
            himg.setMiscInfo  (pimg.miscInfo());
            // Delete PagedImage if HDF5 is used.
            Table::deleteTable (imgName);
          }

        } else {
          // Do the cleaning.
          if (! maskName.empty()) {
            if (maskValue >= 0) {
              PagedImage<float> pimg(imgName);
              maskBlc = handlePos (maskBlc, IPosition(pimg.ndim(), 0));
              maskTrc = handlePos (maskTrc, pimg.shape() - 1);
              imager.boxmask (maskName,
                              maskBlc.asVector(),
                              maskTrc.asVector(),
                              maskValue);
            }
          }
          Timer timer;
          PrecTimer precTimer;
          precTimer.start();
          if (operation == "entropy") {
            imager.mem(operation,                       // algorithm
                       niter,                           // niter
                       sigma,                           // sigma
                       targetFlux,                      // targetflux
                       constrainFlux,                   // constrainflux
                       displayProgress,                 // displayProgress
                       Vector<String>(1, modelName),    // model
                       Vector<Bool>(1, fixed),          // fixed
                       "",                              // complist
                       Vector<String>(1, priorName),    // prior
                       Vector<String>(1, maskName),     // mask
                       Vector<String>(1, restoName),    // restored
                       Vector<String>(1, residName));   // residual
          }
          if (operation == "msmfs") {
            //uInt nterms(2);

            //imager.settaylorterms(nterms,5.95e+07);
            imager.settaylorterms(nterms,RefFreq);
            String scaleMethod;
            Vector<Float> userVector(1); userVector(0)=0;
            convertArray (userVector, userScaleSizes);
            if (userScaleSizes.size() > 1) {
              scaleMethod = "uservector";
            } else {
              scaleMethod = "nscales";
            }
            imager.setscales(scaleMethod, 1, userVector);
            makeEmpty (imager, imgName, 0);
            Directory filee(imgName);

	  
            Vector<String> modelNames(nterms);
            for(Int i=0;i<nterms;++i){
              modelNames(i)="test.img.model.tt"+String::toString(i);
	    
              Directory filee0(modelNames(i));
              File file_model0(modelNames(i));
              if(file_model0.exists()){filee0.removeRecursive();};
              Path model0(modelNames(i)); 
              filee.copy(model0);
            };

            //	  assert(false);

            imager.clean("msmfs",                     // algorithm,
                         niter,                         // niter
                         gain,                          // gain
                         threshold,                     // threshold
                         displayProgress,               // displayProgress
                         //Vector<String>(1, modelName),  // model
                         modelNames,
                         Vector<Bool>(1, fixed),        // fixed
                         "",                            // complist
                         Vector<String>(1, maskName),   // mask
                         Vector<String>(1, restoName),  // restored
                         Vector<String>(1, residName),  // residual
                         Vector<String>(1, psfName));   // psf
	  
          }
          else {
            Vector<String> modelNames(2);
            modelNames[0]="model.main";
            modelNames[1]="model.outlier";
            File Dir_masks_file("JAWS_products");
            Directory Dir_masks("JAWS_products");
            if(Dir_masks_file.exists()){
              Dir_masks.removeRecursive();
            }
            if(Use_masks){
              Dir_masks.create();
            }
            // Vector<String> Namelist(5);
            // Namelist[0]=".spheroid_cut_im";
            // Namelist[1]=".residual";
            // Namelist[2]=".residual.corr";
            // Namelist[3]=".restored";
            // Namelist[4]=".restored.corr";
            // //Namelist[5]="0.avgpb";
            // for(uInt i=0; i<Namelist.size(); ++i){
            //   String avgpb_name(imgName + Namelist[i]);
            //   File avgpb_name_file(avgpb_name);
            //   Directory avgpb_name_dir(avgpb_name);
            //   cout<<avgpb_name<<endl;
            //   if(avgpb_name_file.exists()){
            //     cout<<"... remove"<<endl;
            //     avgpb_name_dir.removeRecursive();
            //   }
            // }
            // Regex rx1 (Regex::fromPattern ("*.avgpb"));
            // Vector<String> avgpbfiles;
            // avgpbfiles=Directory::find(rx1);

            imager.clean(operation,                     // algorithm,
                         niter,                         // niter
                         gain,                          // gain
                         threshold,                     // threshold
                         displayProgress,               // displayProgress
                         Vector<String>(1, modelName),  // model
                         //  modelNames,
                         Vector<Bool>(1, fixed),        // fixed
                         "",                            // complist
                         Vector<String>(1, maskName),   // mask
                         Vector<String>(1, restoName),  // restored
                         Vector<String>(1, residName),  // residual
                         Vector<String>(1, psfName));   // psf

          }
          // Do the final correction for primary beam and spheroidal.
          bool ApplyElement = (StepApplyElement>0);
          Bool doRestored = (niter>0);
          correctImages (restoName, modelName, residName, imgName, imager,
                         ApplyElement, UseWSplit, doRestored);
          precTimer.stop();
          timer.show ("clean");
          ///        imager.showTimings (cout, precTimer.getReal());
          // Convert result to fits if needed.
          if (! fitsName.empty()) {
            String error;
            PagedImage<float> img(restoName);
            if (! ImageFITSConverter::ImageToFITS (error,
                                                   img,
                                                   fitsName,
                                                   64,         // memoryInMB
                                                   preferVelocity)) {
              throw AipsError(error);
            }
          }
        }
      }
    }
  }catch (AipsError& x) {
    cout << x.getMesg() << endl;
    return 1;
  }
  cout << "awimager normally ended" << endl;
  return 0;
}

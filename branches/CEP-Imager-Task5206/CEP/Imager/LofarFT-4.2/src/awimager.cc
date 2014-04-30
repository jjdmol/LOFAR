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
#include <LofarFT/Imager.h>
#include <LofarFT/Operation.h>
#include <Common/ParameterSet.h>
#include <Common/InputParSet.h>
#include <Common/SystemUtil.h>
#include <Common/Exception.h>
#include <LofarFT/Package__Version.h>
#include <Common/Version.h>

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

#include <boost/algorithm/string/join.hpp>

using namespace casa;

// Use a terminate handler that can produce a backtrace.
LOFAR::Exception::TerminateHandler t(LOFAR::Exception::terminate);

IPosition handlePos (const IPosition& pos, const IPosition& def);
Matrix<Bool> readMueller (const String& str, String stokes, Bool grid);
void printHelp(Int argc, char** argv);
void applyFactors (PagedImage<Float>& image, const Array<Float>& factors);
void correctImages (
  const String& restoName, 
  const String& modelName,
  const String& residName, 
  const String& imgName,
  Imager&,
  Bool CorrectElement, 
  Bool CorrectWPlanes, 
  Bool doRestored);

int main (Int argc, char** argv)
{
  INIT_LOGGER(LOFAR::basename(string(argv[0])));
  vector<string> operations = LOFAR::LofarFT::OperationFactory::instance().registeredClassIds();
  
  LOFAR::InputParSet initial_inputs;
  
  LOFAR::Version version_info = LOFAR::LofarFTVersion().getInfo();
  string version = version_info.version() + " revision=" + version_info.packageRevision();
  if (version_info.nrChangedFiles() != "0") 
  {
    if (version_info.nrChangedFiles() == "1") 
    {
       version += " (1 locally modified file)";
    }
    else
    {
      version += " (" + version_info.nrChangedFiles() + " locally modified files)";
    }
  }
     
  initial_inputs.setVersion(version);
  initial_inputs.create ("operation", "image",
              "Operation (" + boost::algorithm::join(operations, ",") + ")",
              "string");
  try
  {
    initial_inputs.readArguments (argc, argv);
  }
  catch (...)
  {
  }

  string parsetname;
  if (argc<=1 || string(argv[1])=="--help" || string(argv[1])=="-help" || string(argv[1])=="help") {
    printHelp(argc,argv);
    exit(0);
  }
  else {
    parsetname=argv[1];
  }

  LOFAR::ParameterSet parset(parsetname);
  parset.adoptArgv(argc,argv);
  
  String operation_name = parset.getString("operation");
  LOFAR::LofarFT::Operation *operation = LOFAR::LofarFT::OperationFactory::instance().create(operation_name,parset);
  if (!operation)
  {
    cout << "Unknown operation: " << operation_name << endl;
    return 1;
  }

  operation->init();

  vector<string> unused = parset.unusedKeys();
  if (! unused.empty()) {
     cout<< "*** WARNING: the following parset keywords were not used ***"<<endl;
     cout<< "             maybe they are misspelled"<<endl;
     cout<< "    " << unused << endl;
  }
  
  try 
  {
    operation->run();
  }
  
  catch (AipsError& x) 
  {
    cout << x.getMesg() << endl;
    return 1;
  }
  cout << "awimager normally ended" << endl;
  return 0;

  try {
    LOFAR::InputParSet inputs;
    // define the input structure
    // Get the input specification.
    Bool fixed          = inputs.getBool("fixed");
    Bool UseLIG         = inputs.getBool("UseLIG");
    Bool UseWSplit         = inputs.getBool("UseWSplit");
    Bool UseEJones      = inputs.getBool("UseEJones");
    Bool MakeDirtyCorr  = inputs.getBool("MakeDirtyCorr");
    Bool applyIonosphere = inputs.getBool("applyIonosphere");
    String parmdbname = inputs.getString("parmdbname");
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
    String antenna   = inputs.getString("antenna");
    
    //Double FillFactor= 1.;//inputs.getDouble("FillFactor");

    // Check and interpret input values.
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
//     if (imgName.empty()) {
//       imgName = msName;
//       imgName.gsub (Regex("\\..*"), "");
//       imgName.gsub (Regex(".*/"), "");
//       imgName += '-' + stokes + '-' + mode + String::toString(img_nchan)
// 	+ ".img";
//     }
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
//       phaseCenter = readDirection (phasectr);
    }
    operation.downcase();
    ASSERTSTR (operation=="empty" || 
                  operation=="image" || 
                  operation=="csclean"|| 
                  operation=="msmfs"||
                  operation=="predict"||
                  operation=="psf"||
                  operation=="mfclark"||
                  operation=="multiscale", 
                  "Unknown operation");
    ///AlwaysAssertExit (operation=="empty" || operation=="image" || operation=="hogbom" || operation=="clark" || operation=="csclean" || operation=="multiscale" || operation =="entropy");
    IPosition maskBlc, maskTrc;
    Quantity threshold;
    Quantity sigma;
    Quantity targetFlux;
    Bool doClean = (operation != "empty"  &&  operation != "image"&&  operation != "psf");
    if (doClean) {
//       maskBlc = readIPosition (mstrBlc);
//       maskTrc = readIPosition (mstrTrc);
//       threshold = readQuantity (threshStr);
//       sigma = readQuantity (sigmaStr);
//       targetFlux = readQuantity (targetStr);
    }
    Bool doPSF =(operation=="psf");
    if(doPSF==true){
      operation="csclean";
      niter=0;
    }
    // Get axis specification from filter.
    Quantity bmajor, bminor, bpa;
//     readFilter (filter, bmajor, bminor, bpa);

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
    params.define ("parmdbname", parmdbname);
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
    
    params.define ("ATerm", inputs.getString("ATerm"));
    params.define ("ATermPython.module", inputs.getString("ATermPython.module"));
    params.define ("ATermPython.class", inputs.getString("ATermPython.class"));
    
    
    //params.define ("FillFactor", FillFactor);
    
    LOFAR::LofarFT::Imager imager(ms, params, parset);

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
                    antenna,                        // antnames
                    String(),                       // spwstring
                    uvdist,                       // uvdist
                    String(),                       // scan
                    String(),                      // intent
                    String(),                       // obs
                    True);                         // useModelCol

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
//         makeEmpty (imager, imgName, fieldid);
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
//             makeEmpty (imager, imgName, 0);
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
                         Vector<String>(1, modelName),  // model
                           //  modelNames,
                         Vector<Bool>(1, fixed),        // fixed
                         "",                            // complist
                         Vector<String>(1, maskName),   // mask
                         Vector<String>(1, restoName),  // restored
                         Vector<String>(1, residName),  // residual
                         Vector<String>(1, psfName));   // psf

          }
          imager.restoreImages(Vector<String>(1, restoName));
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


void printHelp(Int argc, char** argv) {
  cout<<"Usage: awimager file.parset [parsetkeys]"<<endl;
  if (argc>2) {
    String operation_name = argv[2];
    LOFAR::ParameterSet emptyparset;
    LOFAR::LofarFT::Operation *operation =
        LOFAR::LofarFT::OperationFactory::instance().create(operation_name,emptyparset);
    if (!operation)
    {
      cout << "Unknown operation: " << operation_name << endl;
      return;
    }
    cout<<"  Additional usage on "<<argv[2]<<endl;
    operation->showHelp(cout,operation_name);
  }
}

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

void correctImages (
  const String& restoName, 
  const String& modelName,
  const String& residName, 
  const String& imgName,
  Imager&,
  Bool CorrectElement, 
  Bool CorrectWPlanes, 
  Bool doRestored)
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
//   Matrix<Float> avgPB = LOFAR::LofarFT::ConvolutionFunction::getAveragePB(imgName+"0");
//   Matrix<Float> spheroidCut = LOFAR::LofarFT::ConvolutionFunction::getSpheroidCut(imgName+"0");
  Matrix<Float> avgPB = LOFAR::LofarFT::ConvolutionFunction::getAveragePB(imgName);
  Matrix<Float> spheroidCut = LOFAR::LofarFT::ConvolutionFunction::getSpheroidCut(imgName);

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

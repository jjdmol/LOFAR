#include <fstream>

#include <uvplot/UVPDataAtom.h>

#include <aips/aips.h>
#include <aips/MeasurementSets/MeasurementSet.h>
#include <aips/MeasurementSets/MSMainColumns.h>
#include <aips/MeasurementSets/MSPolColumns.h>
#include <aips/Arrays/Vector.h>
#include <aips/Tables/ExprNode.h>
#include <aips/Tables/TableColumn.h>



//==============>>>  slot_readMeasurementSet  <<<==============

void readMeasurementSet(const std::string& msName,
                        const std::string& uvpName)
{
  std::cout << "Converting " << msName << " to " << uvpName << std::endl;

  std::ofstream  out(uvpName.c_str());

  MeasurementSet ms(msName);
  MSAntenna      AntennaTable(ms.antenna());
  MSField        FieldTable(ms.field());

  

  Table        msTable(msName);

  ROArrayColumn<Complex> DataColumn    (msTable, "DATA");
  ROScalarColumn<Double> TimeColumn    (msTable, "TIME");
  ROScalarColumn<Int>    Antenna1Column(msTable, "ANTENNA1");
  ROScalarColumn<Int>    Antenna2Column(msTable, "ANTENNA2");
  ROScalarColumn<Int>    FieldColumn   (msTable, "FIELD_ID");
  ROArrayColumn<Bool>    FlagColumn    (msTable, "FLAG");
  ROScalarColumn<Double> ExposureColumn(msTable, "EXPOSURE");

  unsigned int NumPolarizations = DataColumn(0).shape()[0];
  unsigned int NumChannels      = DataColumn(0).shape()[1];
  unsigned int NumSelected      = msTable.nrow();

  MSPolarization           PolarizationTable(ms.polarization());
  ROArrayColumn<Int>       PolTypeColumn(PolarizationTable, "CORR_TYPE");

  std::vector<int>         PolType(NumPolarizations);
  std::vector<UVPDataAtom>       Atoms(NumPolarizations);
  std::vector<UVPDataAtomHeader> Headers(NumPolarizations);
  
  for(unsigned int i = 0; i < NumPolarizations; i++) {
    IPosition Pos(1,0);
    Pos[0] = i;
    PolType[i] = PolTypeColumn(0)(Pos);
    Headers[i] = UVPDataAtomHeader(0, 0);
    Headers[i].itsCorrelationType = UVPDataAtomHeader::Correlation(PolType[i]);
    Atoms[i]   = UVPDataAtom(NumChannels, Headers[i]);
    Atoms[i].setHeader(Headers[i]);
  }


  for(unsigned int i = 0; i < NumSelected; i++) {
    
    bool           DeleteData;
    Array<Complex> DataArray(DataColumn(i));
    const Complex* Data = DataArray.getStorage(DeleteData);

    bool           DeleteFlag;
    Array<Bool>    FlagArray(FlagColumn(i));
    const bool*    Flag = FlagArray.getStorage(DeleteFlag);
    
    for(unsigned int j = 0; j < NumChannels; j++) {
      for(unsigned int k = 0; k < NumPolarizations; k++) {      
        Atoms[k].setData(j, *Data++);
        Atoms[k].setFlag(j, *Flag++);
      }
    }

    for(unsigned int k = 0; k < NumPolarizations; k++) {
      Headers[k].itsTime         = TimeColumn(i);
      Headers[k].itsAntenna1     = Antenna1Column(i);
      Headers[k].itsAntenna2     = Antenna2Column(i);
      Headers[k].itsExposureTime = ExposureColumn(i);
      Headers[k].itsFieldID      = FieldColumn(i);
      Headers[k].sortAntennae();
      Atoms[k].setHeader(Headers[k]);
      Atoms[k].store(out);
    }      
    
    DataArray.freeStorage(Data, DeleteData); 
    FlagArray.freeStorage(Flag, DeleteFlag);
    if(i % (NumSelected/90) == 0) {
      std::cout << "." << std::flush;
    }
  }
  std::cout << "done" << std::endl;
}


int main(int argc, char* argv[])
{
  readMeasurementSet(argv[1], argv[2]);
  return 0;
}

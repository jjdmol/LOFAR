// Copyright Notice

// $ID


#if !defined(UVPDATAHEADER_H)
#define UVPDATAHEADER_H



class UVPDataHeader
{
 public:

  UVPDataHeader(int                  correlation       = 0,
                int                  numberOfBaselines = 0,
                int                  numberOfTimeslots = 0,
                int                  numberOfChannels  = 0,
                int                  fieldID           = 0,
                const std::string&  fieldName         = "" );

  int        itsCorrelation;
  int        itsNumberOfBaselines;
  int        itsNumberOfTimeslots;
  int        itsNumberOfChannels;
  int        itsFieldID;
  std::string itsFieldName;
  

 protected:
 private:
};



#endif //UVPDATAHEADER_H

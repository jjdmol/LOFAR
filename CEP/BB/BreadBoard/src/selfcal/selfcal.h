#ifndef __SELFCAL_H
#define __SELFCAL_H

class parameter
{
      char * name;
      double value;
      double delta;
   public:
      void setName(char * newname);
      const char * getName(void)const;
      void setValue(double newvalue);
      const double getValue(void)const;
      void setDelta(double newdelta);
      const double getDelta(void)const;
};


#endif // __SELFCAL_H


class arr
{
public:
      float* get()
      {
         a = new float[3];
         a[0]=12.34;
         a[1]=56.78;
         a[2]=90.12;
         
         return a;
      }
      void set(int,float[]);
private:
      float *a;      
};

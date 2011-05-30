#include <stdio.h>
#include <stdlib.h>

#define N_SUBBANDS (128)

const double pi = 3.14159265395;
const double c  = 3E8;
const double bw_subband = 256E3;
const int    n_subbands = N_SUBBANDS;
const int    coeff_width = 16;

struct packet {
  unsigned char cmd;
  unsigned char seq_nr;
  unsigned char length[2];
  unsigned char mem_select;
  unsigned char coeff[N_SUBBANDS*8];
} packet;

void calc(double theta, double phi, double x, double y, double z, int index, char * coeff)
{
  int i,j;
  double scale = (1<<(coeff_width-2));
  double a0=2*pi*(x*sin(phi)*cos(theta)+y*sin(phi)*cos(theta)+z*cos(phi))/c;
  short a[4];
  
  for(i=0;i<n_subbands;i++) {
    double freq = (i+0.5)*bw_subband;
    int cosa = cos(a0*freq)*scale;
    int sina = sin(a0*freq)*scale;
    
    switch(index) {
        case 0: a[0] = cosa; a[1]= -sina; a[2] = 0;    a[3] = 0;
          break;
        case 1: a[0] = sina; a[1]=  cosa; a[2] = 0;    a[3] = 0;
          break;
        case 2: a[0] = 0   ; a[1]=     0; a[2] = cosa; a[3] = -sina;
          break;
        case 3: a[0] = 0   ; a[1]=     0; a[2] = sina; a[3] =  cosa;
          break;
    }
    for(j=0;j<4;j++) {
      coeff[8*i+2*j+0] = (a[j]>>0) & 0xFF;
      coeff[8*i+2*j+1] = (a[j]>>8) & 0xFF;
    }

/*
    printf("%i %4x %4x %4x %4x\n",i,
        ((unsigned int)a[0]) & 0xffff,
        ((unsigned int)a[1]) & 0xffff,
        ((unsigned int)a[2]) & 0xffff,
        ((unsigned int)a[3]) & 0xffff);
*/
  }
}

void write_hex(char *fnam, unsigned char *buffer, int size)
{
  int i,n_bytes,index,record_type,data,checksum;

  FILE *f;
  f=fopen(fnam,"wt");
  for(i=0;i<size;i++) {
    n_bytes=1;
    index=i;
    record_type=0;
    data=buffer[i];
    checksum  = n_bytes+record_type;
    checksum += (index & 255) + ((index >> 8) & 255);
    checksum += ( data & 255); // & ((data  >> 8) & 255);
    checksum  = (256-(checksum & 255)) & 255;
    fprintf(f,":%02x%04x%02x%02x%02x\n",
               n_bytes,index,record_type,data,checksum);
  }
  fprintf(f,":00000001ff\n");
  fclose(f);
}

void write_data(char *fnam, unsigned char * buffer, int size)
{
  int i;
  FILE *f;
  f=fopen(fnam,"wt");
  for(i=0;i<size;i++) {
    fprintf(f,"%i\n",buffer[i]);
  }
  fclose(f);
}

int main(int argc, char *argv[])
{
  double phi,theta,x,y,z;
  int antenna,direction;
  char fnam[80];

  printf("antenna=");    scanf("%lf",&antenna);
  printf("phi  =");      scanf("%lf",&phi);
  printf("theta=");      scanf("%lf",&theta);
  printf("dist =");      scanf("%lf",&x);
  y=z=0.0;
  packet.cmd = 0xFF;
  packet.length[0] = (sizeof(packet) >> 8) & 0xFF;
  packet.length[1] = (sizeof(packet) >> 0) & 0xFF;
  for(direction=0;direction<4;direction++)
  {
    packet.mem_select = (antenna) << 4 + direction & 0x0F;
    calc(theta,phi,x,y,z,direction,packet.coeff);
    sprintf(fnam,"bf_coeff_%i.cmd",direction);
    write_data(fnam,(char *)&packet,sizeof(packet));
    sprintf(fnam,"bf_coeff_%i.hex",direction);
    write_hex(fnam,(char *)packet.coeff,sizeof(packet.coeff));
  }

  return 0;
}  


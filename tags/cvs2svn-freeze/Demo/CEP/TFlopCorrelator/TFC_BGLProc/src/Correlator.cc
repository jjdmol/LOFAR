#define ELEMENTS 38
#define SAMPLES  780
#define POLAS    2
#define CHANNELS 4

// #include <hummer_builtin.h>

void _correlator(const __complex__ float input[ELEMENTS][POLAS][SAMPLES][CHANNELS], 
		 __complex__ double *out) 
{

//   __alignx(8, &input);
//   __alignx(16, out);

  __complex__ double c0xx = 0, c0xy = 0, c0yx = 0, c0yy = 0;
  __complex__ double c1xx = 0, c1xy = 0, c1yx = 0, c1yy = 0;
  __complex__ double c2xx = 0, c2xy = 0, c2yx = 0, c2yy = 0;
  __complex__ double c3xx = 0, c3xy = 0, c3yx = 0, c3yy = 0;

  const __complex__ float *in_a0x, *in_a1x, *in_a2x, *in_a3x;
  const __complex__ float *in_a0y, *in_a1y, *in_a2y, *in_a3y;
  const __complex__ float *in_b0x, *in_b1x, *in_b2x, *in_b3x;
  const __complex__ float *in_b0y, *in_b1y, *in_b2y, *in_b3y;

  for (int a = 0; a < ELEMENTS; a++) {
    for (int b = 0; b <= a; b++) {

      in_a0x = &input[a][0][0][0];
      in_b0x = &input[b][0][0][0];

      in_b0y = &input[b][1][0][0];
      in_a0y = &input[a][1][0][0];

      for (int s = 0; s < SAMPLES; s++) {

	// unrolled in this loop are 2 polarisations and 
	// 4 channels
	
	c0xx += *in_a0x * ~(*in_b0x); // XX   Channel 1
	in_a1x = in_a0x+1;
	c0xy += *in_a0x * ~(*in_b0y); // XY
	in_b1x = in_b0x+1;
	in_a0x = &input[a][0][s][0];
	c0yx += *in_a0y * ~(*in_b0x); // YX
	in_a1y = in_a0y+1;
	in_b0x = &input[b][0][s][0];
	c0yy += *in_a0y * ~(*in_b0y); // YY
	in_b1y = in_b0y+1;
	in_a0y = &input[a][1][s][0];
	in_b0y = &input[b][1][s][0];

	c1xx += *in_a1x * ~(*in_b1x); // XX   Channel 2
	in_a2x = in_a1x+1;
	c1xy += *in_a1x * ~(*in_b1y); // XY
	in_b2x = in_b1x+1;
	c1yx += *in_a1y * ~(*in_b1x); // YX
	in_a2y = in_a1y+1;
	c1yy += *in_a1y * ~(*in_b1y); // YY
	in_b2y = in_b1y+1;

	c2xx += *in_a2x * ~(*in_b2x); // XX   Channel 3
	in_a3x = in_a2x+1;
	c2xy += *in_a2x * ~(*in_b2y); // XY
	in_b3x = in_b2x+1;
	c2yx += *in_a2y * ~(*in_b2x); // YX
	in_a3y = in_a2y+1;
	c2yy += *in_a2y * ~(*in_b2y); // YY
	in_b3y = in_b2y+1;

	c3xx += *in_a3x * ~(*in_b3x); // XX   Channel 4
	c3xy += *in_a3x * ~(*in_b3y); // XY
	c3yx += *in_a3y * ~(*in_b3x); // YX
	c3yy += *in_a3y * ~(*in_b3y); // YY
      }
      *out++ = c0xx;
      *out++ = c0xy;
      *out++ = c0yx;
      *out++ = c0yy;

      *out++ = c1xx;
      *out++ = c1xy;
      *out++ = c1yx;
      *out++ = c1yy;
      
      *out++ = c2xx;
      *out++ = c2xy;
      *out++ = c2yx;
      *out++ = c2yy;
      
      *out++ = c3xx;
      *out++ = c3xy;
      *out++ = c3yx;
      *out++ = c3yy;
    }
  }
}

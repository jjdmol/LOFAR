typedef float2 fcomplex;
typedef float4 fcomplex2;
typedef float8 fcomplex4;

typedef char4  char_complex2;
typedef short4 short_complex2;


fcomplex cmul(fcomplex a, fcomplex b)
{
  return (fcomplex) (a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}


fcomplex cexp(float ang)
{
  return (fcomplex) (native_cos(ang), native_sin(ang));
}

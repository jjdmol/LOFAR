#if !defined ALIGN_H
#define ALIGN_H

template <typename T> inline static bool powerOfTwo(T n)
{
  return (n | (n - 1)) == 2 * n - 1;
}


template <typename T> inline static T align(T value, size_t alignment)
{
#if defined __GNUC__
  if (__builtin_constant_p(alignment) && powerOfTwo(alignment))
    return (value + alignment - 1) & ~(alignment - 1);
  else
#endif
    return (value + alignment - 1) / alignment * alignment;
}

#endif

// \defgroup mathematics Mathematics

// \ingroup mathematics
// \defgroup math_constants Math Constants
// @{

// Ratio of a circle's circumference to its diameter, \f$\pi\f$.
const Real pi = M_PI; // 3.14159265358979323846264338327950288;

// @}


// \ingroup mathematics
// \defgroup elementary_math Elementary Math
// @{

// Return the square of the argument \a x.
template<typename T>
inline T sqr(T x)
{
  return x*x;
}

// @}

#if defined HAVE_BGP

#include <cstring>

namespace LOFAR {
namespace RTCP {

extern "C" {

// all float * must be aligned to 8 bytes

void _add_2_single_precision_vectors(
  /* r3 */ float *dst,
  /* r4 */ const float *src1,
  /* r5 */ const float *src2,
  /* r6 */ unsigned count /* non-zero; multiple of 16 */
);  

void _add_3_single_precision_vectors(
  /* r3 */ float *dst,
  /* r4 */ const float *src1,
  /* r5 */ const float *src2,
  /* r6 */ const float *src3,
  /* r7 */ unsigned count /* non-zero; multiple of 16 */
);  

void _add_4_single_precision_vectors(
  /* r3 */ float *dst,
  /* r4 */ const float *src1,
  /* r5 */ const float *src2,
  /* r6 */ const float *src3,
  /* r7 */ const float *src4,
  /* r8 */ unsigned count /* non-zero; multiple of 16 */
);  

void _add_5_single_precision_vectors(
  /* r3 */ float *dst,
  /* r4 */ const float *src1,
  /* r5 */ const float *src2,
  /* r6 */ const float *src3,
  /* r7 */ const float *src4,
  /* r8 */ const float *src5,
  /* r9 */ unsigned count /* non-zero; multiple of 16 */
);  

void _add_6_single_precision_vectors(
  /* r3 */ float *dst,
  /* r4 */ const float *src1,
  /* r5 */ const float *src2,
  /* r6 */ const float *src3,
  /* r7 */ const float *src4,
  /* r8 */ const float *src5,
  /* r9 */ const float *src6,
  /* r10 */ unsigned count /* non-zero; multiple of 16 */
);  

#if 0
void _beamform_3beams(
  /* r3 */ fcomplex *dst,
  /* r4 */ size_t dst_beam_stride,
  /* r5 */ unsigned nr_stations,
  /* r6 */ unsigned nr_times,
  /* r7 */ fcomplex *samples,
  /* r8 */ unsigned station_samples_stride,
  /* r9 */ fcomplex *weights,
  /* r10 */ unsigned station_weights_stride
);

void _beamform_1station_1beam(
  /* r3 */ fcomplex *complex_voltages,
  /* r4 */ const fcomplex *samples,
  /* r5 */ const fcomplex *weight,
  /* r6 */ unsigned nr_times
);

void _beamform_2stations_1beam(
  /* r3 */ fcomplex *complex_voltages,
  /* r4 */ const fcomplex *samples,
  /* r5 */ unsigned samples_stride,
  /* r6 */ const fcomplex *weight,
  /* r7 */ unsigned weights_stride,
  /* r8 */ unsigned nr_times
);
#endif

void *_beamform_3stations_6beams(
  /* r3 */ fcomplex *complex_voltages,
  /* r4 */ unsigned complex_voltages_stride,
  /* r5 */ const fcomplex *samples,
  /* r6 */ unsigned samples_stride,
  /* r7 */ const fcomplex *weights,
  /* r8 */ unsigned weights_stride,
  /* r9 */ unsigned nr_times,
  /* r10 */ bool first_time // if !first_time, then add to complex_voltages
);

void *_beamform_up_to_6_stations_and_3_beams(
  /* r3 */ fcomplex *complex_voltages,
  /* r4 */ unsigned complex_voltages_stride,
  /* r5 */ const fcomplex *samples,
  /* r6 */ unsigned samples_stride,
  /* r7 */ const fcomplex *weights,
  /* r8 */ unsigned weights_stride,
  /* r9 */ unsigned nr_times,
  /* r10 */ bool first_time, // if !first_time, then add to complex_voltages
  /* 8(r1) */ unsigned nr_stations, // 1-6
  /* 12(r1) */ unsigned nr_beams // 1-3
);

#if 0
void _beamform_4stations_3beams(
  /* r3 */ fcomplex *complex_voltages,
  /* r4 */ unsigned complex_voltages_stride,
  /* r5 */ const fcomplex *samples,
  /* r6 */ unsigned samples_stride,
  /* r7 */ const fcomplex *weights,
  /* r8 */ unsigned weights_stride,
  /* r9 */ unsigned nr_times
);

void _beamform_6beams_2times(
  /* r3 */ fcomplex *dst,
  /* r4 */ size_t dst_beam_stride,
  /* r5 */ unsigned nr_stations,
  /* r6 */ fcomplex *samples,
  /* r7 */ unsigned station_samples_stride,
  /* r8 */ fcomplex *weights,
  /* r9 */ unsigned station_weights_stride
);
#endif

} // extern "C"

// Similar functions that do not need or have an ASM version

// defined just to aid the use of macros
static inline void _add_1_single_precision_vectors(
  float *dst,
  const float *src1,
  unsigned count /* non-zero; multiple of 16 */
) {
  // nothing to add, so just copy the values
  memcpy( dst, src1, count * sizeof(float) );
}

static inline void _add_7_single_precision_vectors(
  float *dst,
  const float *src1,
  const float *src2,
  const float *src3,
  const float *src4,
  const float *src5,
  const float *src6,
  const float *src7,
  unsigned count /* non-zero; multiple of 16 */
) {
  _add_4_single_precision_vectors( dst, src1, src2, src3, src4, count );
  _add_4_single_precision_vectors( dst, dst,  src5, src6, src7, count );
}

} // namespace LOFAR::RTCP
} // namespace LOFAR

#endif

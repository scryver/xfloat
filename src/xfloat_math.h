// TODO(michiel): Maybe pass in a context with scratch space??
internal void   xf_truncate(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_floor(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_ceil(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_round(u32 elemCount, u32 *src, u32 *dst);

// NOTE(michiel): Good starting value for iterations is elemCount / 2, but could maybe be even lower
internal void   xf_square_root(u32 elemCount, u32 *src, u32 *dst, u32 iterations = 8);
internal void   xf_log(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_log2(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_log10(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_exp(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_exp2(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_exp10(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_pow(u32 elemCount, u32 *base, u32 *power, u32 *dst);
internal void   xf_powi(u32 elemCount, u32 *base, u32 *intPower, u32 *dst);

internal void   xf_sin(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_cos(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_tan(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_cot(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_asin(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_acos(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_atan(u32 elemCount, u32 *src, u32 *dst);
// TODO(michiel): Or do we want to deviate the arguments and do a atan2(x, y) ??
internal void   xf_atan2(u32 elemCount, u32 *y, u32 *x, u32 *dst);

internal void   xf_sinh(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_cosh(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_tanh(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_asinh(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_acosh(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_atanh(u32 elemCount, u32 *src, u32 *dst);

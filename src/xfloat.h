// TODO(michiel): Add return code to void functions, with over/underflow flags.
// This will allow for checks on calculations if you want them.

#ifndef XFLOAT_STICKY_BIT
#define XFLOAT_STICKY_BIT    0
#endif

#define XFLOAT_SIGN_MASK     0x80000000
#define XFLOAT_EXPONENT_MASK 0x7FFFFFFF
#define XFLOAT_MAX_EXPONENT  0x00010000  // NOTE(michiel): Configurable up to (and including) 0x40000000
#define XFLOAT_MAX_EXP_MASK  0x0000FFFF
// TODO(michiel): Make it so that infinity is on MAX_EXPONENT - 1, that way we can take it up to 0x80000000
#define XFLOAT_SIGN_EXP_IDX  0
#define XFLOAT_MANTISSA_IDX  1 // NOTE(michiel): x[XFLOAT_MANTISSA_IDX] should always be 0 in normalized form

#define XFLOAT_SIGN_EXPONENT(s, e) (((s) & XFLOAT_SIGN_MASK) | ((e) & XFLOAT_EXPONENT_MASK))

#define XFLOAT_EXP_BIAS      (XFLOAT_MAX_EXPONENT >> 1)
#define XFLOAT_EXP_ONE       (XFLOAT_EXP_BIAS + 1)

#define XFLOAT_MAX_BITS(c)   (((c)-(XFLOAT_MANTISSA_IDX + 1))*32)
#define XFLOAT_MAX_DEC(c)    ((XFLOAT_MAX_BITS(c)*8)/27)

#define XFLOAT_HIGH_BIT      0x80000000

//
// NOTE(michiel): External interface
//
internal void   xf_clear(u32 elemCount, u32 *x);
internal void   xf_copy(u32 elemCount, u32 *src, u32 *dst);
internal void   xf_infinite(u32 elemCount, u32 *x);
internal void   xf_negate(u32 elemCount, u32 *x);
internal void   xf_absolute(u32 elemCount, u32 *x);
internal void   xf_max_value(u32 elemCount, u32 *x);

internal b32    xf_is_infinite(u32 elemCount, u32 *x);

internal void   xf_minimum(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);   // NOTE(michiel): dst = src1 < src2 ? src1 : src2
internal void   xf_maximum(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);   // NOTE(michiel): dst = src1 > src2 ? src1 : src2
internal void   xf_add(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);       // NOTE(michiel): dst = src1 + src2
internal void   xf_sub(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);       // NOTE(michiel): dst = src1 - src2
internal void   xf_mul(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);       // NOTE(michiel): dst = src1 * src2
internal void   xf_div(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);       // NOTE(michiel): dst = src1 / src2
internal s64    xf_integer_fraction(u32 elemCount, u32 *src, u32 *fraction); // NOTE(michiel): integer part returned, remaining fraction in fraction array
internal s32    xf_compare(u32 elemCount, u32 *src1, u32 *src2);             // NOTE(michiel): {-1: src1 < src2, 0: src1 = src2, 1: src1 > src2}
internal void   xf_from_f32(u32 elemCount, f32 f, u32 *x);
internal void   xf_from_f64(u32 elemCount, f64 f, u32 *x);
internal void   xf_from_s32(u32 elemCount, s32 s, u32 *x);
internal void   xf_from_s64(u32 elemCount, s64 s, u32 *x);
internal void   xf_print_raw(u32 elemCount, u32 *x, b32 newLine);
// TODO(michiel): xf_from_u32/u64

// NOTE(michiel): Use xfloat_string.cpp for these extra conversion functions (they need the constants file as well).
//internal void   xf_from_string(u32 elemCount, String string, u32 *x);
//internal String string_from_xf(u32 elemCount, u32 *x, u32 digits, u32 maxDataCount, u8 *data);
//internal void   xf_print(u32 elemCount, u32 *x, u32 digits)
//
//

// NOTE(michiel): Use xfloat_math.cpp for extra math functions (they need the constants file as well).
//
//

// NOTE(michiel): Used to complete certain functions
enum XFloatCompletion
{
    XFloatCompletion_None      = 0x00,
    XFloatCompletion_Copy      = 0x01,
    XFloatCompletion_Overflow  = 0x02,
    XFloatCompletion_Underflow = 0x04,
};

internal u32
xf_get_sign(u32 elemCount, u32 *x)
{
    u32 result = x[XFLOAT_SIGN_EXP_IDX] & XFLOAT_SIGN_MASK;
    return result;
}

internal void
xf_make_positive(u32 elemCount, u32 *x)
{
    x[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
}

internal void
xf_make_negative(u32 elemCount, u32 *x)
{
    x[XFLOAT_SIGN_EXP_IDX] |= XFLOAT_SIGN_MASK;
}

internal void
xf_flip_sign(u32 elemCount, u32 *x)
{
    x[XFLOAT_SIGN_EXP_IDX] ^= XFLOAT_SIGN_MASK;
}

internal void
xf_multiply_sign(u32 elemCount, u32 *a, u32 *b, u32 *c)
{
    c[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
    c[XFLOAT_SIGN_EXP_IDX] |= (a[XFLOAT_SIGN_EXP_IDX] ^ b[XFLOAT_SIGN_EXP_IDX]) & XFLOAT_SIGN_MASK;
}

internal u32
xf_get_exponent(u32 elemCount, u32 *x)
{
    u32 result = x[XFLOAT_SIGN_EXP_IDX] & XFLOAT_EXPONENT_MASK;
    return result;
}

internal s64
xf_unbiased_exponent(u32 elemCount, u32 *x)
{
    // TODO(michiel): Make this return a s32 and fix up all code usage
    return (s64)xf_get_exponent(elemCount, x) - (s64)XFLOAT_EXP_BIAS;
}

internal void
xf_set_exponent(u32 elemCount, u32 *x, u32 exponent)
{
    // TODO(michiel): Rename to biased exponent maybe
    x[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_EXPONENT_MASK;
    x[XFLOAT_SIGN_EXP_IDX] |= (exponent & XFLOAT_EXPONENT_MASK);
}

internal void
xf_clear(u32 elemCount, u32 *x)
{
    copy_single(sizeof(u32)*elemCount, 0, x);
}

internal void
xf_copy(u32 elemCount, u32 *src, u32 *dst)
{
    copy(sizeof(u32)*elemCount, src, dst);
}

internal void
xf_copy_extend(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): This will copy all of src to dst as well as an extra guard 0,
    // so make sure you have space in dst for the extra word.
    xf_copy(elemCount, src, dst);
    dst[elemCount] = 0;
}

internal void
xf_infinite(u32 elemCount, u32 *dst)
{
    xf_set_exponent(elemCount, dst, XFLOAT_MAX_EXPONENT);
    u32 *x = dst + XFLOAT_MANTISSA_IDX;
    *x++ = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX - 1; ++index)
    {
        *x++ = U32_MAX;
    }
}

internal void
xf_negate(u32 elemCount, u32 *x)
{
    xf_flip_sign(elemCount, x);
}

internal void
xf_absolute(u32 elemCount, u32 *x)
{
    xf_make_positive(elemCount, x);
}

internal void
xf_max_value(u32 elemCount, u32 *x)
{
    xf_make_positive(elemCount, x);
    xf_set_exponent(elemCount, x, XFLOAT_MAX_EXPONENT - 1);
    u32 *d = x + XFLOAT_MANTISSA_IDX;
    *d++ = 0;
    for (u32 index = 0; index < (elemCount - XFLOAT_MANTISSA_IDX - 1); ++index)
    {
        *d++ = U32_MAX;
    }
}

internal b32
xf_is_infinite(u32 elemCount, u32 *x)
{
    b32 result = xf_get_exponent(elemCount, x) >= XFLOAT_MAX_EXPONENT;
    return result;
}

internal void
xf_naive_div2(u32 elemCount, u32 *x)
{
    // NOTE(michiel): This is _not_ save for any value. The exponent is not checked!
    xf_set_exponent(elemCount, x, xf_get_exponent(elemCount, x) - 1);
}

internal void
xf_naive_mul2(u32 elemCount, u32 *x)
{
    // NOTE(michiel): This is _not_ save for any value. The exponent is not checked!
    xf_set_exponent(elemCount, x, xf_get_exponent(elemCount, x) + 1);
}



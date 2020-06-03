#ifndef XFLOAT_STICKY_BIT
#define XFLOAT_STICKY_BIT    0
#endif

#define XFLOAT_SIGN_MASK     0x80000000
#define XFLOAT_EXPONENT_MASK 0x7FFFFFFF
#define XFLOAT_MAX_EXPONENT  0x00010000  // NOTE(michiel): Configurable up to (and including) 0x40000000
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
internal void   xf_infinite(u32 elemCount, u32 *src);
internal void   xf_negate(u32 elemCount, u32 *src);
internal void   xf_absolute(u32 elemCount, u32 *src);

internal void   xf_add(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);       // NOTE(michiel): dst = src1 + src2
internal void   xf_subtract(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);  // NOTE(michiel): dst = src1 - src2
internal void   xf_multiply(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);  // NOTE(michiel): dst = src1 * src2
internal void   xf_divide(u32 elemCount, u32 *src1, u32 *src2, u32 *dst);    // NOTE(michiel): dst = src1 / src2
internal s64    xf_integer_fraction(u32 elemCount, u32 *src, u32 *fraction); // NOTE(michiel): integer part returned, remaining fraction in fraction array
internal s32    xf_compare(u32 elemCount, u32 *src1, u32 *src2);             // NOTE(michiel): {-1: src1 < src2, 0: src1 = src2, 1: src1 > src2}
internal void   xf_from_f32(u32 elemCount, f32 f, u32 *x);
internal void   xf_from_f64(u32 elemCount, f64 f, u32 *x);

// NOTE(michiel): Use xfloat_string.cpp for these extra conversion functions (they need the constants file as well).
//internal void   xf_from_string(u32 elemCount, String string, u32 *x);
//internal String string_from_xf(u32 elemCount, u32 *x, u32 digits, u32 maxDataCount, u8 *data);
//
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
    return (s64)xf_get_exponent(elemCount, x) - (s64)XFLOAT_EXP_BIAS;
}

internal void
xf_set_exponent(u32 elemCount, u32 *x, u32 exponent)
{
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
xf_infinite(u32 elemCount, u32 *src)
{
    xf_set_exponent(elemCount, src, XFLOAT_MAX_EXPONENT);
    u32 *x = src + XFLOAT_MANTISSA_IDX;
    *x++ = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX - 1; ++index)
    {
        *x++ = U32_MAX;
    }
}

internal void
xf_negate(u32 elemCount, u32 *src)
{
    xf_flip_sign(elemCount, src);
}

internal void
xf_absolute(u32 elemCount, u32 *src)
{
    xf_make_positive(elemCount, src);
}



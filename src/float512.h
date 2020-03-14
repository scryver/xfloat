#define F512_ELEMENT_COUNT  16

struct f512
{
    u32 e[F512_ELEMENT_COUNT];
};

internal f512
F512(f32 f)
{
    f512 result;
    xf_from_f32(F512_ELEMENT_COUNT, f, result.e);
    return result;
}

internal f512
F512(String s)
{
    f512 result;
    xf_from_string(F512_ELEMENT_COUNT, s, result.e);
    return result;
}

internal String
string_from_f512(f512 *a, u32 digits, u32 maxDataCount, u8 *data)
{
    String result = string_from_xf(F512_ELEMENT_COUNT, a->e, digits, maxDataCount, data);
    return result;
}

internal f512
operator -(f512 &a)
{
    f512 result = a;
    xf_negate(F512_ELEMENT_COUNT, result.e);
    return result;
}

internal f512 &
operator +=(f512 &a, f512 &b)
{
    xf_add(F512_ELEMENT_COUNT, a.e, b.e, a.e);
    return a;
}

internal f512
operator +(f512 &a, f512 &b)
{
    f512 result;
    xf_add(F512_ELEMENT_COUNT, a.e, b.e, result.e);
    return result;
}

internal f512 &
operator -=(f512 &a, f512 &b)
{
    xf_subtract(F512_ELEMENT_COUNT, a.e, b.e, a.e);
    return a;
}

internal f512
operator -(f512 &a, f512 &b)
{
    f512 result;
    xf_subtract(F512_ELEMENT_COUNT, a.e, b.e, result.e);
    return result;
}

internal f512 &
operator *=(f512 &a, f512 &b)
{
    xf_multiply(F512_ELEMENT_COUNT, a.e, b.e, a.e);
    return a;
}

internal f512
operator *(f512 &a, f512 &b)
{
    f512 result;
    xf_multiply(F512_ELEMENT_COUNT, a.e, b.e, result.e);
    return result;
}

internal f512 &
operator /=(f512 &a, f512 &b)
{
    xf_divide(F512_ELEMENT_COUNT, a.e, b.e, a.e);
    return a;
}

internal f512
operator /(f512 &a, f512 &b)
{
    f512 result;
    xf_divide(F512_ELEMENT_COUNT, a.e, b.e, result.e);
    return result;
}



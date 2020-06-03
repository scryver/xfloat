global u32 gXF_FloorBitmask[] = {
    0xFFFFFFFF,
    0xFFFFFFFE,
    0xFFFFFFFC,
    0xFFFFFFF8,
    0xFFFFFFF0,
    0xFFFFFFE0,
    0xFFFFFFC0,
    0xFFFFFF80,
    0xFFFFFF00,
    0xFFFFFE00,
    0xFFFFFC00,
    0xFFFFF800,
    0xFFFFF000,
    0xFFFFE000,
    0xFFFFC000,
    0xFFFF8000,
    0xFFFF0000,
    0xFFFE0000,
    0xFFFC0000,
    0xFFF80000,
    0xFFF00000,
    0xFFE00000,
    0xFFC00000,
    0xFF800000,
    0xFF000000,
    0xFE000000,
    0xFC000000,
    0xF8000000,
    0xF0000000,
    0xE0000000,
    0xC0000000,
    0x80000000,
    0x00000000,
};

internal void
xf_floor(u32 elemCount, u32 *src, u32 *dst)
{
    i_expect(xf_get_exponent(elemCount, src) < XFLOAT_MAX_EXPONENT);

    u32 buf[elemCount];
    xf_copy(elemCount, src, buf);

    if (xf_get_exponent(elemCount, buf))
    {
        xf_copy(elemCount, buf, dst);
        s64 exponent = xf_unbiased_exponent(elemCount, buf);

        if (exponent <= 0)
        {
            if (xf_get_sign(elemCount, buf))
            {
                xf_copy(elemCount, gXF_One, dst);
                xf_make_negative(elemCount, dst);
            }
            else
            {
                xf_clear(elemCount, dst);
            }
        }
        else
        {
            exponent = XFLOAT_MAX_BITS(elemCount) - exponent;

            u32 *p = dst + elemCount - 1;
            while (exponent >= 32)
            {
                *p-- = 0;
                exponent -= 32;
            }

            i_expect(exponent >= 0);
            i_expect(exponent < 32);
            *p &= gXF_FloorBitmask[exponent];

            if (xf_get_sign(elemCount, buf))
            {
                if (xf_compare(elemCount, buf, dst) != 0) {
                    xf_subtract(elemCount, dst, gXF_One, dst);
                }
            }
        }
    }
    else
    {
        xf_clear(elemCount, dst);
    }
}

internal void
xf_round(u32 elemCount, u32 *src, u32 *dst)
{
    u32 z[elemCount];
    u32 f[elemCount];

    xf_floor(elemCount, src, z);
    xf_subtract(elemCount, src, z, f);

    s32 r = xf_compare(elemCount, f, gXF_Half);
    if (r > 0)
    {
        xf_add(elemCount, gXF_One, z, z);
    }
    else if (r == 0)
    {
        if (!xf_get_sign(elemCount, z))
        {
            xf_add(elemCount, gXF_One, z, z);
        }
    }

    xf_copy(elemCount, z, dst);
}

// TODO(michiel): Generate these
global u32 gXF_SquareCoef2[16] = {
    XFLOAT_SIGN_MASK | (XFLOAT_EXP_BIAS - 2),
    0, 0xD14FC42F, 0xE79BA800, 0, 0
};

global u32 gXF_SquareCoef1[16] = {
    XFLOAT_EXP_BIAS,
    0, 0xE3E3C2AE, 0x4C146700, 0, 0
};

global u32 gXF_SquareCoef0[16] = {
    XFLOAT_EXP_BIAS - 1,
    0, 0xA08BDC7D, 0xD5FFE300, 0, 0
};

global u32 gXF_SquareRoot2[16] = {
    XFLOAT_EXP_BIAS + 1,
    0, 0xb504f333,0xf9de6484,0x597d89b3,0x754abe9f,0x1d6f60ba,
    0x893ba84c,0xed17ac85,0x83339915,0x4afc8304,0x3ab8a2c3,0xa8b1fe70,
};

internal void
xf_square_root(u32 elemCount, u32 *src, u32 *dst, u32 iterations = 8)
{
    u32 accum1[elemCount];
    u32 accum2[elemCount];

    if (xf_get_sign(elemCount, src))
    {
        if (xf_compare(elemCount, gXF_Zero, src) == 0)
        {
            xf_clear(elemCount, dst);
            xf_make_negative(elemCount, dst);
        }
        else
        {
            // NOTE(michiel): Imaginary number!
            xf_clear(elemCount, dst);
        }
    }
    else
    {
        if (xf_get_exponent(elemCount, src))
        {
            xf_copy(elemCount, src, accum1);
            s32 exponent = (s32)xf_unbiased_exponent(elemCount, src);
            xf_set_exponent(elemCount, accum1, XFLOAT_EXP_BIAS);

            // NOTE(michiel): y = (x * coef2 + coef1) * x + coef0
            xf_multiply(elemCount, gXF_SquareCoef2, accum1, accum2);
            xf_add(elemCount, gXF_SquareCoef1, accum2, accum2);
            xf_multiply(elemCount, accum1, accum2, accum2);
            xf_add(elemCount, gXF_SquareCoef0, accum2, accum2);

            xf_set_exponent(elemCount, accum2, (exponent / 2) + XFLOAT_EXP_BIAS);

            if (exponent & 0x1)
            {
                xf_multiply(elemCount, gXF_SquareRoot2, accum2, accum2);
            }

            for (u32 index = 0; index < iterations; ++index)
            {
                xf_divide(elemCount, src, accum2, accum1);
                xf_add(elemCount, accum1, accum2, accum2);
                xf_set_exponent(elemCount, accum2, xf_get_exponent(elemCount, accum2) - 1);
            }

            xf_copy(elemCount, accum2, dst);
        }
        else
        {
            xf_clear(elemCount, dst);
        }
    }
}
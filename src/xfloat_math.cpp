// TODO(michiel): Generate these
// These are polynomial coefficients that approximate the square root function
// in the range of [0.5, 1.0].
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

global u32 gXF_TruncateBitMask[] = {
    0xFFFFFFFF,
    0x80000000,
    0xC0000000,
    0xE0000000,
    0xF0000000,
    0xF8000000,
    0xFC000000,
    0xFE000000,
    0xFF000000,
    0xFF800000,
    0xFFC00000,
    0xFFE00000,
    0xFFF00000,
    0xFFF80000,
    0xFFFC0000,
    0xFFFE0000,
    0xFFFF0000,
    0xFFFF8000,
    0xFFFFC000,
    0xFFFFE000,
    0xFFFFF000,
    0xFFFFF800,
    0xFFFFFC00,
    0xFFFFFE00,
    0xFFFFFF00,
    0xFFFFFF80,
    0xFFFFFFC0,
    0xFFFFFFE0,
    0xFFFFFFF0,
    0xFFFFFFF8,
    0xFFFFFFFC,
    0xFFFFFFFE,
};

internal void
xf_truncate(u32 elemCount, u32 *src, u32 *dst)
{
    u32 buf[elemCount];
    xf_copy(elemCount, src, buf);

    u32 totalWords = (elemCount - (XFLOAT_MANTISSA_IDX + 1));
    u32 totalBits = totalWords * sizeof(u32) * 8;
    s64 usefulBits = xf_unbiased_exponent(elemCount, buf);
    u32 useBits = (usefulBits > 0) ? safe_truncate_to_u32(usefulBits) : 0;
    if (useBits > totalBits)
    {
        useBits = totalBits;
    }

    dst[XFLOAT_SIGN_EXP_IDX] = buf[XFLOAT_SIGN_EXP_IDX];
    dst[XFLOAT_MANTISSA_IDX] = 0;

    u32 *s = buf + XFLOAT_MANTISSA_IDX + 1;
    u32 *d = dst + XFLOAT_MANTISSA_IDX + 1;

    u32 useWords = (useBits + 31) / 32;
    if (useWords)
    {
        xf_copy(useWords, s, d);
        // NOTE(michiel): Mask of last useful word
        d[useWords - 1] &= gXF_TruncateBitMask[useBits & 0x1F];
    }
    xf_clear(totalWords - useWords, d + useWords);
}

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
xf_ceil(u32 elemCount, u32 *src, u32 *dst)
{
    xf_floor(elemCount, src, dst);
    if (xf_compare(elemCount, dst, src) < 0)
    {
        xf_add(elemCount, dst, gXF_One, dst);
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

internal void
xf_square_root(u32 elemCount, u32 *src, u32 *dst, u32 iterations /* = 8 */)
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
                xf_multiply(elemCount, gXF_Sqrt2, accum2, accum2);
            }

            for (u32 index = 0; index < iterations; ++index)
            {
                xf_divide(elemCount, src, accum2, accum1);
                xf_add(elemCount, accum1, accum2, accum2);
                xf_naive_div2(elemCount, accum2);
            }

            xf_copy(elemCount, accum2, dst);
        }
        else
        {
            xf_clear(elemCount, dst);
        }
    }
}

internal void
xf_log(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumX[elemCount];
    u32 accumZ[elemCount];
    u32 accumA[elemCount];
    u32 accumB[elemCount];
    u32 accumT[elemCount];
    u32 accumJ[elemCount];

    if (!xf_get_sign(elemCount, src))
    {
        if (xf_get_exponent(elemCount, src))
        {
            /* range reduction: log x = log( 2**ex * m ) = ex * log2 + log m */
            b32 foundAnswer = false;

            xf_copy(elemCount, src, accumX);
            s32 exponent = xf_get_exponent(elemCount, accumX);
            if (exponent == XFLOAT_EXP_ONE)
            {
                // NOTE(michiel): Log 1 = 0
                if (xf_compare(elemCount, src, gXF_One) == 0)
                {
                    xf_clear(elemCount, dst);
                    foundAnswer = true;
                }
            }

            if (!foundAnswer)
            {
                exponent -= XFLOAT_EXP_BIAS;
                xf_set_exponent(elemCount, accumX, XFLOAT_EXP_BIAS);
                /* Adjust range to 1/sqrt(2), sqrt(2) */
                xf_naive_div2(elemCount, gXF_Sqrt2);
                if (xf_compare(elemCount, accumX, gXF_Sqrt2) < 0)
                {
                    --exponent;
                    xf_naive_mul2(elemCount, accumX);
                }
                xf_naive_mul2(elemCount, gXF_Sqrt2);

                xf_add(elemCount, gXF_One, accumX, accumB);
                xf_subtract(elemCount, accumX, gXF_One, accumA);
                if (!xf_get_exponent(elemCount, accumA))
                {
                    xf_clear(elemCount, dst);
                    foundAnswer = true;
                }

                if (!foundAnswer)
                {
                    xf_divide(elemCount, accumA, accumB, dst); /* store (src-1)/(src+1) in dst */

                    xf_multiply(elemCount, dst, dst, accumZ);

                    xf_copy(elemCount, gXF_One, accumA);
                    xf_copy(elemCount, gXF_One, accumB);
                    xf_copy(elemCount, gXF_One, accumJ);

                    do
                    {
                        xf_add(elemCount, gXF_Two, accumJ, accumJ); /* 2 * i + 1 */
                        xf_multiply(elemCount, accumZ, accumA, accumA);
                        xf_divide(elemCount, accumA, accumJ, accumT);
                        xf_add(elemCount, accumT, accumB, accumB);
                    }
                    while(((s32)xf_get_exponent(elemCount, accumB) - (s32)xf_get_exponent(elemCount, accumT)) < XFLOAT_MAX_BITS(elemCount));

                    xf_multiply(elemCount, accumB, dst, dst);
                    xf_naive_mul2(elemCount, dst);
                }

                /* now add log of 2**ex */
                if (exponent != 0)
                {
                    xf_from_s32(elemCount, exponent, accumB);
                    xf_multiply(elemCount, gXF_Log2Lower, accumB, accumT);
                    xf_add(elemCount, accumT, dst, dst);
                    xf_multiply(elemCount, gXF_Log2Upper, accumB, accumT);
                    xf_add(elemCount, accumT, dst, dst);
                }
            }
        }
        else
        {
            // NOTE(michiel): Log of 0
            xf_infinite(elemCount, dst);
            xf_make_negative(elemCount, dst);
        }
    }
    else
    {
        // NOTE(michiel): Domain error
        xf_clear(elemCount, dst);
    }
}

internal void
xf_log2(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): Log2(x) = Log2(e) * Log(x)
#if 0
    u32 logSrc[elemCount];
    u32 accum[elemCount];

    xf_log(elemCount, src, logSrc);
    xf_multiply(elemCount, gXF_Log2eLower, logSrc, dst);
    xf_multiply(elemCount, gXF_Log2eUpper, logSrc, accum);
    xf_add(elemCount, accum, dst, dst);
#else
    xf_log(elemCount, src, dst);
    xf_divide(elemCount, dst, gXF_Log2, dst);
#endif
}

internal void
xf_log10(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): Log10(x) = Log10(e) * Log(x)
    u32 logSrc[elemCount];
    u32 accum[elemCount];

    xf_log(elemCount, src, logSrc);
    xf_multiply(elemCount, gXF_Log10eLower, logSrc, dst);
    xf_multiply(elemCount, gXF_Log10eUpper, logSrc, accum);
    xf_add(elemCount, accum, dst, dst);
}

internal void
xf_exp(u32 elemCount, u32 *src, u32 *dst)
{
    u32 numerator[elemCount];
    u32 denominator[elemCount];
    u32 accum[elemCount];

    /* range reduction theory: x = i + f, 0<=f<1;
     * e^x = e^i * e^f
     * e^i = 2^(i/log 2).
     * Let i/log2 = i1 + f1, 0 <= f1 < 1.
     * Then e^i = 2^i1 * 2^f1, so
     * e^x = 2^i1 * e^(f1 log 2) * e^f.
     */

    /* Catch overflow that might cause an endless recursion below.  */
    // TODO(michiel): Let the max exponent depend on elemCount
    if (xf_get_exponent(elemCount, src) < XFLOAT_EXP_ONE + 15)
    {
        if (xf_get_exponent(elemCount, src))
        {
            xf_copy(elemCount, src, accum);
            xf_divide(elemCount, accum, gXF_Log2, denominator);
            xf_add(elemCount, gXF_Half, denominator, denominator);
            xf_floor(elemCount, denominator, numerator);
            s64 i = xf_integer_fraction(elemCount, numerator, denominator);
            xf_multiply(elemCount, gXF_Log2Upper, numerator, denominator);
            xf_subtract(elemCount, accum, denominator, accum);
            xf_multiply(elemCount, gXF_Log2Lower, numerator, denominator);
            xf_subtract(elemCount, accum, denominator, accum);

            xf_naive_div2(elemCount, accum);
            xf_tanh(elemCount, accum, accum);
            xf_add(elemCount, gXF_One, accum, numerator);        // 1 + tanh(x/2)
            xf_subtract(elemCount, gXF_One, accum, denominator); // 1 - tanh(x/2)
            xf_divide(elemCount, numerator, denominator, dst);   // (1 + tanh(x/2)) / (1 - tanh(x/2))

            i += xf_get_exponent(elemCount, dst);
            if (i > XFLOAT_MAX_EXPONENT)
            {
                // NOTE(michiel): Overflow
                xf_infinite(elemCount, dst);
            }
            else if (i <= 0)
            {
                // NOTE(michiel): Underflow
                xf_clear(elemCount, dst);
            }
            else
            {
                xf_set_exponent(elemCount, dst, i);
            }
        }
        else
        {
            xf_copy(elemCount, gXF_One, dst);
        }
    }
    else
    {
        if (xf_get_sign(elemCount, src))
        {
            // NOTE(michiel): Underflow
            xf_clear(elemCount, dst);
        }
        else
        {
            // NOTE(michiel): Overflow
            xf_infinite(elemCount, dst);
        }
    }
}

internal void
xf_exp2(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): 2^x = e^(x * log(2))
    u32 accum[elemCount];

    xf_multiply(elemCount, src, gXF_Log2, accum);
    xf_exp(elemCount, accum, dst);
}

internal void
xf_exp10(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): 10^x = e^(x * log(10))
    u32 accum[elemCount];

    xf_multiply(elemCount, src, gXF_Log10, accum);
    xf_exp(elemCount, accum, dst);
}

internal void
xf_pow(u32 elemCount, u32 *base, u32 *power, u32 *dst)
{
    u32 accum[elemCount];

    b32 done = false;
    xf_floor(elemCount, power, accum);
    if (xf_compare(elemCount, power, accum) == 0)
    {
        s64 i = xf_integer_fraction(elemCount, power, accum);
        if (i < 0)
        {
            i = -i;
        }
        if (i <= XFLOAT_MAX_EXPONENT)
        {
            xf_powi(elemCount, base, power, dst);
            done = true;
        }
    }

    if (!done)
    {
        xf_log(elemCount, base, accum);
        xf_multiply(elemCount, power, accum, accum);
        xf_exp(elemCount, accum, dst);
    }
}

internal void
xf_powi(u32 elemCount, u32 *base, u32 *intPower, u32 *dst)
{
    u32 accum[elemCount];

    s64 li = xf_integer_fraction(elemCount, intPower, accum);
    xf_copy(elemCount, base, accum);
    u32 signBase = xf_get_sign(elemCount, accum);
    xf_make_positive(elemCount, accum);
    s64 lx = li < 0 ? -li : li;

    // NOTE(michiel): Check for 2^N
    if (xf_compare(elemCount, gXF_Two, accum) == 0)
    {
        xf_copy(elemCount, gXF_Two, dst);
        if (signBase && (lx & 1))
        {
            xf_make_negative(elemCount, dst);
        }
        else
        {
            xf_make_positive(elemCount, dst);
        }

        s64 e = xf_get_exponent(elemCount, accum) + li - 1;
        if (e <= XFLOAT_MAX_EXPONENT)
        {
            if (e <= 0)
            {
                xf_clear(elemCount, dst);
            }
            else
            {
                xf_set_exponent(elemCount, dst, e);
            }
        }
        else
        {
            // NOTE(michiel): Overflow
            xf_infinite(elemCount, dst);
        }
    }
    else
    {
        if (lx == 0x7FFFFFFF)
        {
            // NOTE(michiel): xf_integer_fraction overflowed
            xf_pow(elemCount, base, intPower, dst);
        }
        else
        {
            if (xf_get_exponent(elemCount, base))
            {
                if (li)
                {
                    u32 signPower = 0;
                    if (li < 0)
                    {
                        li = -li;
                        signPower = XFLOAT_SIGN_MASK;
                    }

                    if (li & 1)
                    {
                        xf_copy(elemCount, accum, dst);
                    }
                    else
                    {
                        xf_copy(elemCount, gXF_One, dst);
                        signBase = 0;
                    }

                    li >>= 1;
                    while (li)
                    {
                        xf_multiply(elemCount, accum, accum, accum);
                        if (li & 1)
                        {
                            xf_multiply(elemCount, accum, dst, dst);
                        }
                        li >>= 1;
                    }

                    if (signBase)
                    {
                        xf_negate(elemCount, dst);
                    }

                    if (signPower)
                    {
                        if (xf_get_exponent(elemCount, dst))
                        {
                            xf_divide(elemCount, gXF_One, dst, dst);
                        }
                        else
                        {
                            // NOTE(michiel): Overflow
                            xf_infinite(elemCount, dst);
                        }
                    }
                }
                else
                {
                    xf_copy(elemCount, gXF_One, dst);
                }
            }
            else
            {
                if (li == 0)
                {
                    xf_copy(elemCount, gXF_One, dst);
                }
                else if (li < 0)
                {
                    xf_infinite(elemCount, dst);
                }
                else
                {
                    xf_clear(elemCount, dst);
                }
            }
        }
    }
}

internal void
xf_sin(u32 elemCount, u32 *angle, u32 *dst)
{
    u32 accum1[elemCount];
    u32 accum2[elemCount];
    u32 accum3[elemCount];
    u32 accum4[elemCount];

    u32 sign = xf_get_sign(elemCount, angle);

    xf_copy(elemCount, angle, accum4);
    xf_make_positive(elemCount, accum4);
    /* range reduction to [0, pi/2] */
    xf_copy(elemCount, gXF_PiOver2, accum3);
    xf_divide(elemCount, accum4, accum3, accum1);
    xf_floor(elemCount, accum1, accum4);

    /* accum2 = accum4 - 8 * floor(accum4/8) */
    u32 exponentAccum4 = xf_get_exponent(elemCount, accum4);
    if (exponentAccum4 >= 3)
    {
        xf_set_exponent(elemCount, accum4, exponentAccum4 - 3);
        xf_floor(elemCount, accum4, accum2);
        xf_set_exponent(elemCount, accum2, xf_get_exponent(elemCount, accum2) + 3);
        xf_set_exponent(elemCount, accum4, exponentAccum4);
    }
    else
    {
        xf_clear(elemCount, accum2);
    }

    xf_subtract(elemCount, accum4, accum2, accum2);
    s64 mod = xf_integer_fraction(elemCount, accum2, accum2);

    xf_subtract(elemCount, accum1, accum4, accum2);
    xf_multiply(elemCount, accum2, accum3, accum4);

    mod &= 3;
    if (mod > 1)
    {
        sign = XFLOAT_SIGN_MASK;
    }
    if (mod & 1)
    {
        xf_subtract(elemCount, accum3, accum4, accum4); /* accum4 = 1 - accum4 */
    }

    xf_multiply(elemCount, accum4, accum4, accum3);
    xf_negate(elemCount, accum3);

    xf_copy(elemCount, gXF_One, accum1);
    xf_copy(elemCount, gXF_One, accum2);
    xf_copy(elemCount, gXF_One, dst);

    /* power series */
    do
    {
        xf_add(elemCount, gXF_One, accum1, accum1);     /* accum1 += 1      */
        xf_divide(elemCount, accum2, accum1, accum2);   /* accum2 /= accum1 */
        xf_add(elemCount, gXF_One, accum1, accum1);     /* accum1 += 1 */
        xf_divide(elemCount, accum2, accum1, accum2);   /* accum2 /= accum1 */
        xf_multiply(elemCount, accum3, accum2, accum2); /* accum2 *= accum3 */
        xf_add(elemCount, accum2, dst, dst);            /* dst += accum2 */
    }
    while((s32)(xf_get_exponent(elemCount, dst) - xf_get_exponent(elemCount, accum2)) < XFLOAT_MAX_BITS(elemCount));

    xf_multiply(elemCount, accum4, dst, dst);

    dst[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
    dst[XFLOAT_SIGN_EXP_IDX] |= sign;
}

internal void
xf_cos(u32 elemCount, u32 *angle, u32 *dst)
{
    u32 accum[elemCount];
    xf_copy(elemCount, gXF_PiOver2, accum);
    xf_subtract(elemCount, accum, angle, accum);
    xf_sin(elemCount, accum, dst);
}

internal void
xf_tanh(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumE[elemCount];
    u32 accumR[elemCount];
    u32 accumJ[elemCount];
    u32 accumX[elemCount];
    u32 accumM2[elemCount];

    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumR);
    xf_make_positive(elemCount, accumR);

    if (xf_compare(elemCount, accumR, gXF_One) >= 0)
    {
        /* This subroutine is used by the exponential function routine.
         * tanh(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
         * Note qexp() calls qtanh, but with an argument less than (1 + log 2)/2.
     */
        xf_exp(elemCount, accumR, accumE);
        xf_divide(elemCount, gXF_One, accumE, accumR);
        xf_subtract(elemCount, accumE, accumR, accumX);
        xf_add(elemCount, accumE, accumR, accumJ);
        xf_divide(elemCount, accumX, accumJ, dst);
    }
    else
    {
        xf_copy(elemCount, gXF_Two, accumM2);
        xf_negate(elemCount, accumM2);

        /* Adjust loop count for convergence to working precision.  */
        u32 n = XFLOAT_MAX_BITS(elemCount) / 9 + 1;
        s64 lj = 2 * n + 1;
        xf_from_s64(elemCount, lj, accumJ);

        xf_copy(elemCount, accumJ, accumE);
        xf_multiply(elemCount, src, src, accumX);

        /* continued fraction */

        for (u32 i = 0; i < n; ++i)
        {
            xf_divide(elemCount, accumX, accumE, accumR);
            xf_add(elemCount, accumM2, accumJ, accumJ);
            xf_add(elemCount, accumR, accumJ, accumE);
        }

        xf_divide(elemCount, src, accumE, dst);
    }

    if (sign)
    {
        xf_make_negative(elemCount, dst);
    }
}


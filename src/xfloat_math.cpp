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
    // NOTE(michiel): Everything from XFLOAT_EXP_ONE onward is useful
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
    else
    {
        // NOTE(michiel): Zero out all
        xf_set_exponent(elemCount, dst, 0);
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
                    xf_sub(elemCount, dst, gXF_One, dst);
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
    xf_floor(elemCount, src, dst);            // Y = FLOOR(X)
    if (xf_compare(elemCount, dst, src) < 0)  // Y < X
    {
        xf_add(elemCount, dst, gXF_One, dst); // Y += 1
    }
}

internal void
xf_round(u32 elemCount, u32 *src, u32 *dst)
{
    u32 z[elemCount];

    xf_floor(elemCount, src, z);
    xf_sub(elemCount, src, z, dst);

    s32 r = xf_compare(elemCount, dst, gXF_Half);
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
    if (xf_get_sign(elemCount, src))                           // X < 0
    {
        if (xf_compare(elemCount, gXF_Zero, src) == 0)         // X = -0
        {
            xf_clear(elemCount, dst);                          // Y = 0
            xf_make_negative(elemCount, dst);                  // Y = -0
        }
        else
        {
            // NOTE(michiel): Imaginary number!
            xf_clear(elemCount, dst);                          // Y = 0
        }
    }
    else
    {
        if (xf_get_exponent(elemCount, src))                   // X > 0
        {
            u32 accum1[elemCount];
            u32 accum2[elemCount];

            xf_copy(elemCount, src, accum1);                                       // A = X
            s32 exponent = (s32)xf_unbiased_exponent(elemCount, src);              // E = EXPONENT(X)
            xf_set_exponent(elemCount, accum1, XFLOAT_EXP_BIAS);                   // A => RANGE (-1, 1)

            // NOTE(michiel): y = (x * coef2 + coef1) * x + coef0
            xf_mul(elemCount, gXF_SquareRootCoef2, accum1, accum2);           // B = A * C2  | C2*X
            xf_add(elemCount, gXF_SquareRootCoef1, accum2, accum2);                // B += C1     | C2*X + C1
            xf_mul(elemCount, accum1, accum2, accum2);                        // B *= A      | C2*X^2 + C1*X
            xf_add(elemCount, gXF_SquareRootCoef0, accum2, accum2);                // B += C0     | C2*X^2 + C1*X + C0

            xf_set_exponent(elemCount, accum2, (exponent / 2) + XFLOAT_EXP_BIAS);  // B => RANGE(-SQRT(X), SQRT(X))

            if (exponent & 0x1)                                                    // Exponent is odd
            {
                xf_mul(elemCount, gXF_Sqrt2, accum2, accum2);                 // B *= SQRT(2)
            }

            for (u32 index = 0; index < iterations; ++index)
            {
                // NOTE(michiel): X / ASQRT(X) should approach SQRT(X)
                xf_div(elemCount, src, accum2, accum1);                         // A = X / B    | X / ASQRT(X)
                xf_add(elemCount, accum1, accum2, accum2);                         // B = A + B    | X / ASQRT(X) + ASQRT(X)
                xf_naive_div2(elemCount, accum2);                                  // B /= 2       | (X / ASQRT(X) + ASQRT(X)) / 2
            }

            xf_copy(elemCount, accum2, dst);                                       // Y = B
        }
        else
        {
            xf_clear(elemCount, dst);                        // Y = 0   | X == 0
        }
    }
}

internal void
xf_log(u32 elemCount, u32 *src, u32 *dst)
{
    if (!xf_get_sign(elemCount, src))
    {
        if (xf_get_exponent(elemCount, src))
        {
            /* range reduction: log x = log( 2**ex * m ) = ex * log2 + log m */
            b32 foundAnswer = false;

            u32 accumX[elemCount];

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
                u32 accumB[elemCount];
                u32 accumT[elemCount];

                exponent -= XFLOAT_EXP_BIAS;
                xf_set_exponent(elemCount, accumX, XFLOAT_EXP_BIAS);          // X => RANGE (-1, 1)
                /* Adjust range to 1/sqrt(2), sqrt(2) */
                xf_copy(elemCount, gXF_Sqrt2, accumB);                        // B = SQRT(2)
                xf_naive_div2(elemCount, accumB);                             // B >>= 1
                if (xf_compare(elemCount, accumX, accumB) < 0)                // X < B
                {
                    --exponent;
                    xf_naive_mul2(elemCount, accumX);                         // X <<= 1
                }

                xf_add(elemCount, accumX, gXF_One, accumB);                   // B = X + 1
                xf_sub(elemCount, accumX, gXF_One, accumX);              // X = X - 1
                if (!xf_get_exponent(elemCount, accumX))
                {
                    xf_clear(elemCount, dst);
                    foundAnswer = true;
                }

                if (!foundAnswer)
                {
                    u32 accumZ[elemCount];
                    u32 accumJ[elemCount];

                    xf_div(elemCount, accumX, accumB, dst);                // Y = X / B      | (X - 1) / (X + 1)

                    xf_mul(elemCount, dst, dst, accumZ);                 // Z = Y^2

                    xf_copy(elemCount, gXF_One, accumX);                      // X = 1
                    xf_copy(elemCount, gXF_One, accumB);                      // B = 1
                    xf_copy(elemCount, gXF_One, accumJ);                      // J = 1

                    // NOTE(michiel): B = 1 + Y^2/3 + Y^4/5 + Y^6/7 + ...
                    do
                    {
                        xf_add(elemCount, gXF_Two, accumJ, accumJ);           // J += 2       /* 2 * i + 1 */
                        xf_mul(elemCount, accumZ, accumX, accumX);       // X *= Z       | X * Y^2
                        xf_div(elemCount, accumX, accumJ, accumT);         // T = X / J    | (X * Y^2) / J
                        xf_add(elemCount, accumT, accumB, accumB);            // B += T       | ...
                    }
                    while((s32)(xf_get_exponent(elemCount, accumB) - xf_get_exponent(elemCount, accumT)) < XFLOAT_MAX_BITS(elemCount));

                    xf_mul(elemCount, accumB, dst, dst);                 // Y *= B
                    xf_naive_mul2(elemCount, dst);                            // Y <<= 1
                }

                /* now add log of 2**ex */
                if (exponent != 0)
                {
                    xf_from_s32(elemCount, exponent, accumB);                 // B = XF(exponent)
                    xf_mul(elemCount, gXF_Log2Lower, accumB, accumT);    // T = B * LOG(2)[high]
                    xf_add(elemCount, accumT, dst, dst);                      // Y += T
                    xf_mul(elemCount, gXF_Log2Upper, accumB, accumT);    // T = B * LOG(2)[low]
                    xf_add(elemCount, accumT, dst, dst);                      // Y += T
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
    xf_mul(elemCount, gXF_Log2eLower, logSrc, dst);
    xf_mul(elemCount, gXF_Log2eUpper, logSrc, accum);
    xf_add(elemCount, accum, dst, dst);
#else
    xf_log(elemCount, src, dst);                // Y = LOG(X)
    xf_div(elemCount, dst, gXF_Log2, dst);   // Y /= LOG(2)        | LOG(X) / LOG(2)
#endif
}

internal void
xf_log10(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): Log10(x) = Log10(e) * Log(x)
#if 0
    u32 accum[elemCount];

    xf_log(elemCount, src, accum);
    xf_mul(elemCount, gXF_Log10eLower, accum, dst);
    xf_mul(elemCount, gXF_Log10eUpper, accum, accum);
    xf_add(elemCount, accum, dst, dst);
#else
    xf_log(elemCount, src, dst);               // Y = LOG(X)
    xf_div(elemCount, dst, gXF_Log10, dst); // Y /= LOG(10)        | LOG(X) / LOG(10)
#endif
}

internal void
xf_exp(u32 elemCount, u32 *src, u32 *dst)
{
    /* range reduction theory: x = i + f, 0<=f<1;
     * e^x = e^i * e^f
     * e^i = 2^(i/log 2).
     * Let i/log2 = i1 + f1, 0 <= f1 < 1.
     * Then e^i = 2^i1 * 2^f1, so
     * e^x = 2^i1 * e^(f1 log 2) * e^f.
     */

    /* Catch overflow that might cause an endless recursion below.  */
    // TODO(michiel): Let the max exponent depend on XFLOAT_MAX_EXPONENT
    if (xf_get_exponent(elemCount, src) < XFLOAT_EXP_ONE + 15)
    {
        if (xf_get_exponent(elemCount, src))
        {
            u32 numerator[elemCount];
            u32 denominator[elemCount];
            u32 accum[elemCount];

            xf_copy(elemCount, src, accum);                                 // A = X
            xf_div(elemCount, accum, gXF_Log2, denominator);             // D = A / LOG(2)   | X / LOG(2)
            xf_add(elemCount, gXF_Half, denominator, denominator);          // D += 0.5         | 0.5 + X / LOG(2)
            xf_floor(elemCount, denominator, numerator);                    // N = FLOOR(D)     | FLOOR(0.5 + X / LOG(2))
            s64 i = xf_integer_fraction(elemCount, numerator, denominator); // I = INT(N)
            xf_mul(elemCount, gXF_Log2Upper, numerator, denominator);  // D = N * LOG(2)[high]
            xf_sub(elemCount, accum, denominator, accum);              // A -= D
            xf_mul(elemCount, gXF_Log2Lower, numerator, denominator);  // D = N * LOG(2)[low]
            xf_sub(elemCount, accum, denominator, accum);              // A -= D           | X - LOG(2) * FLOOR(0.5 + X / LOG(2))

            xf_naive_div2(elemCount, accum);                                // A >>= 1          | X / 2
            xf_tanh(elemCount, accum, accum);                               // A = TANH(A)      | TANH(X / 2)
            xf_add(elemCount, gXF_One, accum, numerator);                   // N = 1 + A        | 1 + TANH(X / 2)
            xf_sub(elemCount, gXF_One, accum, denominator);            // D = 1 - A        | 1 - TANH(X / 2)
            xf_div(elemCount, numerator, denominator, dst);              // Y = N / D        | (1 + TANH(X/2)) / (1 - TANH(X/2))

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
    xf_mul(elemCount, src, gXF_Log2, dst);
    xf_exp(elemCount, dst, dst);
}

internal void
xf_exp10(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): 10^x = e^(x * log(10))
    xf_mul(elemCount, src, gXF_Log10, dst);
    xf_exp(elemCount, dst, dst);
}

internal void
xf_pow(u32 elemCount, u32 *base, u32 *power, u32 *dst)
{
    // NOTE(michiel): Y = B ^ P
    u32 accum[elemCount];

    b32 done = false;
    xf_floor(elemCount, power, accum);                         // A = FLOOR(P)
    if (xf_compare(elemCount, power, accum) == 0)              // P == A
    {
        s64 i = xf_integer_fraction(elemCount, power, accum);
        if (i < 0)
        {
            i = -i;
        }
        if (i <= XFLOAT_MAX_EXPONENT)
        {
            xf_powi(elemCount, base, power, dst);              // Y = POWI(B, P) | P is INT
            done = true;
        }
    }

    if (!done)
    {
        xf_log(elemCount, base, accum);                        // A = LOG(B)
        xf_mul(elemCount, power, accum, accum);           // A *= P
        xf_exp(elemCount, accum, dst);                         // Y = EXP(A)   | e^(power * log(base))
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
                        xf_mul(elemCount, accum, accum, accum);
                        if (li & 1)
                        {
                            xf_mul(elemCount, accum, dst, dst);
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
                            xf_div(elemCount, gXF_One, dst, dst);
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
xf_sin(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accum1[elemCount];
    u32 accum2[elemCount];
    u32 accum4[elemCount];

    u32 sign = xf_get_sign(elemCount, src);                                            // sign = src < 0

    xf_copy(elemCount, src, accum4);                                                   // vvvvvv
    xf_make_positive(elemCount, accum4);                                               // accum4 = absolute(src);
    /* range reduction to [0, pi/2] */
    xf_div(elemCount, accum4, gXF_PiOver2, accum1);                                 // accum1 = accum4 / (Pi/2)
    xf_floor(elemCount, accum1, accum4);                                               // accum4 = floor(accum1)

    /* accum2 = accum4 - 8 * floor(accum4/8) */
    u32 exponentAccum4 = xf_get_exponent(elemCount, accum4);
    if (exponentAccum4 >= 3)
    {
        xf_set_exponent(elemCount, accum4, exponentAccum4 - 3);
        xf_floor(elemCount, accum4, accum2);
        xf_set_exponent(elemCount, accum2, xf_get_exponent(elemCount, accum2) + 3);
        xf_set_exponent(elemCount, accum4, exponentAccum4);
        xf_sub(elemCount, accum4, accum2, accum2);
    }
    else
    {
        xf_copy(elemCount, accum4, accum2);
    }

    s64 mod = xf_integer_fraction(elemCount, accum2, accum2);

    xf_sub(elemCount, accum1, accum4, accum2);
#if 1
    xf_mul(elemCount, accum2, gXF_PiOver2, accum4);
#else
    xf_multiply(elemCount, accum2, gXF_PiOver2Upper, accum4);
    xf_multiply(elemCount, accum2, gXF_PiOver2Lower, accum2);
    xf_add(elemCount, accum2, accum4, accum4);
#endif

    mod &= 3;
    if (mod > 1)
    {
        sign = XFLOAT_SIGN_MASK;
    }
    if (mod & 1)
    {
        xf_sub(elemCount, gXF_PiOver2, accum4, accum4); /* accum4 = 1 - accum4 */
    }

    u32 accum3[elemCount];
    xf_mul(elemCount, accum4, accum4, accum3);
    xf_negate(elemCount, accum3);

    xf_copy(elemCount, gXF_One, accum1);
    xf_copy(elemCount, gXF_One, accum2);
    xf_copy(elemCount, gXF_One, dst);

    /* power series */
    do
    {
        xf_add(elemCount, gXF_One, accum1, accum1);     /* accum1 += 1      */
        xf_div(elemCount, accum2, accum1, accum2);   /* accum2 /= accum1 */
        xf_add(elemCount, gXF_One, accum1, accum1);     /* accum1 += 1 */
        xf_div(elemCount, accum2, accum1, accum2);   /* accum2 /= accum1 */
        xf_mul(elemCount, accum3, accum2, accum2); /* accum2 *= accum3 */
        xf_add(elemCount, accum2, dst, dst);            /* dst += accum2 */
    }
    while((s32)(xf_get_exponent(elemCount, dst) - xf_get_exponent(elemCount, accum2)) < XFLOAT_MAX_BITS(elemCount));

    xf_mul(elemCount, accum4, dst, dst);

    dst[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
    dst[XFLOAT_SIGN_EXP_IDX] |= sign;
}

internal void
xf_cos(u32 elemCount, u32 *src, u32 *dst)
{
    xf_sub(elemCount, gXF_PiOver2, src, dst);
    xf_sin(elemCount, dst, dst);
}

internal void
xf_tan(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumX3[elemCount];
    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumX3);
    xf_make_positive(elemCount, accumX3);

    // NOTE(michiel): Range reduction to +/- pi/2
    u32 accumE[elemCount];
    xf_add(elemCount, accumX3, gXF_PiOver2, accumE);
    xf_div(elemCount, accumE, gXF_Pi, accumE);
    xf_floor(elemCount, accumE, accumE);
    xf_mul(elemCount, accumE, gXF_Pi, accumE);
    xf_sub(elemCount, accumX3, accumE, accumX3);

    u32 accumJ[elemCount];
    u32 n = XFLOAT_MAX_BITS(elemCount) / 8;
    s64 li = 2 * n + 1;
    xf_from_s64(elemCount, li, accumJ);
    xf_copy(elemCount, accumJ, accumE);

    u32 accumX2[elemCount];
    xf_mul(elemCount, accumX3, accumX3, accumX2);
    xf_negate(elemCount, accumX2);

    // NOTE(michiel): Continued fraction expansion
    u32 accumR[elemCount];
    for (u32 i = 0; i < n; ++i)
    {
        xf_div(elemCount, accumX2, accumE, accumR);
        xf_sub(elemCount, accumJ, gXF_Two, accumJ);
        xf_add(elemCount, accumR, accumJ, accumE);
    }

    xf_div(elemCount, accumX3, accumE, dst);
    if (sign)
    {
        xf_negate(elemCount, dst);
    }
}

internal void
xf_asin(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumA[elemCount];
    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumA);
    xf_make_positive(elemCount, accumA);

    if (xf_compare(elemCount, accumA, gXF_One) <= 0)
    {
        u32 accumZSqr[elemCount];

        u32 flag = 0;
        if (xf_compare(elemCount, accumA, gXF_Half) > 0)
        {
            xf_sub(elemCount, gXF_Half, accumA, accumZSqr);
            xf_add(elemCount, gXF_Half, accumZSqr, accumZSqr);
            if (xf_get_exponent(elemCount, accumZSqr))
            {
                xf_naive_div2(elemCount, accumZSqr);
            }
            xf_square_root(elemCount, accumZSqr, accumA);
            flag = 1;
        }
        else
        {
            xf_mul(elemCount, accumA, accumA, accumZSqr);
        }

        xf_sub(elemCount, gXF_One, accumZSqr, accumZSqr);
        xf_square_root(elemCount, accumZSqr, accumZSqr);
        xf_div(elemCount, accumA, accumZSqr, accumZSqr);
        xf_atan(elemCount, accumZSqr, dst);

        if (flag)
        {
            xf_naive_mul2(elemCount, dst);
            xf_sub(elemCount, gXF_PiOver2, dst, dst);
        }

        dst[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
        dst[XFLOAT_SIGN_EXP_IDX] |= sign;
    }
    else
    {
        // NOTE(michiel): Domain error
        xf_clear(elemCount, dst);
    }
}

internal void
xf_acos(u32 elemCount, u32 *src, u32 *dst)
{
    xf_copy(elemCount, src, dst);
    xf_make_positive(elemCount, dst);
    if (xf_compare(elemCount, dst, gXF_One) > 0)
    {
        // NOTE(michiel): Domain error
        xf_clear(elemCount, dst);
    }
    else
    {
        xf_asin(elemCount, src, dst);
        xf_sub(elemCount, gXF_PiOver2, dst, dst);
    }
}

internal void
xf_atan(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumX[elemCount];
    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumX);
    xf_make_positive(elemCount, accumX);

    u32 accumY[elemCount];
    if (xf_compare(elemCount, accumX, gXF_Tan3PiOver8) > 0)
    {
        xf_copy(elemCount, gXF_PiOver2, accumY);

        xf_div(elemCount, gXF_One, accumX, accumX);
        xf_negate(elemCount, accumX);
    }
    else if (xf_compare(elemCount, accumX, gXF_TanPiOver8) > 0)
    {
        xf_sub(elemCount, accumX, gXF_One, accumY);
        xf_add(elemCount, accumX, gXF_One, accumX);
        xf_div(elemCount, accumY, accumX, accumX);

        xf_copy(elemCount, gXF_PiOver2, accumY);
        xf_naive_div2(elemCount, accumY);
    }
    else
    {
        xf_clear(elemCount, accumY);
    }

    u32 accumZ[elemCount];
    xf_mul(elemCount, accumX, accumX, accumZ);
    if (xf_get_exponent(elemCount, accumZ))
    {
        u32 accumA[elemCount];
        u32 accumB[elemCount];
        u32 accumJ[elemCount];

        u32 i = 2 * XFLOAT_MAX_BITS(elemCount) / 9;
        u32 j = 2 * i + 1;
        xf_from_s64(elemCount, j, accumJ);
        xf_copy(elemCount, accumJ, accumB);

        while (j > 1)
        {
            s64 nsq = i * i;
            xf_from_s64(elemCount, nsq, accumA);
            xf_multiply_int(elemCount, accumA, accumZ, accumA);
            xf_div(elemCount, accumA, accumB, accumB);
            j -= 2;
            --i;
            xf_sub(elemCount, accumJ, gXF_Two, accumJ);
            xf_add(elemCount, accumJ, accumB, accumB);
        }

        xf_div(elemCount, accumX, accumB, dst);
    }
    else
    {
        xf_copy(elemCount, accumX, dst);
    }

    xf_add(elemCount, accumY, dst, dst);

    if (sign)
    {
        xf_negate(elemCount, dst);
    }
}

// TODO(michiel): Or do we want to deviate the arguments and do a atan2(x, y) ??
internal void
xf_atan2(u32 elemCount, u32 *y, u32 *x, u32 *dst)
{
    s32 quadrant = 0;

    if (xf_get_sign(elemCount, x) && xf_get_exponent(elemCount, x))
    {
        quadrant = 2;
    }
    if (xf_get_sign(elemCount, y) && xf_get_exponent(elemCount, y))
    {
        quadrant |= 1;
    }

    if (xf_get_exponent(elemCount, x) <= 1)
    {
        // NOTE(michiel): Zero x
        if (quadrant & 0x01)
        {
            // NOTE(michiel): Negative y
            xf_copy(elemCount, gXF_PiOver2, dst);
            xf_negate(elemCount, dst);
        }
        else if (xf_get_exponent(elemCount, y) <= 1)
        {
            // NOTE(michiel): Zero y
            xf_clear(elemCount, dst);
        }
        else
        {
            xf_copy(elemCount, gXF_PiOver2, dst);
        }
    }
    else if (xf_get_exponent(elemCount, y) <= 1)
    {
        // NOTE(michiel): Zero y
        if (quadrant & 0x02)
        {
            // NOTE(michiel): Negative x
            xf_copy(elemCount, gXF_Pi, dst);
        }
        else
        {
            xf_clear(elemCount, dst);
        }
    }
    else
    {
        u32 accumW[elemCount];

        switch (quadrant)
        {
            default:
            case 0:
            case 1: {
                xf_clear(elemCount, accumW);
            } break;

            case 2: {
                xf_copy(elemCount, gXF_Pi, accumW);
            } break;

            case 3: {
                xf_copy(elemCount, gXF_Pi, accumW);
                xf_negate(elemCount, accumW);
            } break;
        }

        // NOTE(michiel): dst = w + atan(y/x)
        xf_div(elemCount, y, x, dst);
        xf_atan(elemCount, dst, dst);
        xf_add(elemCount, accumW, dst, dst);
    }
}

internal void
xf_sinh(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumZ[elemCount];

    u32 exponent = xf_get_exponent(elemCount, src);
    if (exponent <= 1)
    {
        // NOTE(michiel): Src is zero
        xf_clear(elemCount, dst);
    }
    else if (exponent < XFLOAT_EXP_BIAS)
    {
        u32 srcSqr[elemCount];
        xf_mul(elemCount, src, src, srcSqr);

        u32 accumN[elemCount];
        u32 accumF[elemCount];

        xf_copy(elemCount, gXF_One, accumZ);
        xf_copy(elemCount, gXF_One, accumF);
        xf_copy(elemCount, gXF_One, accumN);

        // NOTE(michiel): This will loop as long as F has some influence on Z
        // TODO(michiel): One more step and round?
        do
        {
            xf_add(elemCount, gXF_One, accumN, accumN);       // ++N
            xf_div(elemCount, accumF, accumN, accumF);     // F /= N
            xf_add(elemCount, gXF_One, accumN, accumN);       // ++N
            xf_div(elemCount, accumF, accumN, accumF);     // F /= N
            xf_mul(elemCount, srcSqr, accumF, accumF);   // F *= X
            xf_add(elemCount, accumZ, accumF, accumZ);        // Z += F
        } while ((s32)(xf_get_exponent(elemCount, accumZ) - xf_get_exponent(elemCount, accumF)) < XFLOAT_MAX_BITS(elemCount));

        xf_mul(elemCount, src, accumZ, dst);
    }
    else
    {
        xf_exp(elemCount, src, accumZ);
        xf_div(elemCount, gXF_One, accumZ, dst);
        xf_sub(elemCount, accumZ, dst, dst);
        xf_naive_div2(elemCount, dst);
    }
}

internal void
xf_cosh(u32 elemCount, u32 *src, u32 *dst)
{
    // NOTE(michiel): cosh(x) = (e^x + 1 / e^x) / 2

    u32 accum[elemCount];

    xf_exp(elemCount, src, accum);
    xf_div(elemCount, gXF_One, accum, dst);
    xf_add(elemCount, accum, dst, dst);
    if (xf_get_exponent(elemCount, dst))
    {
        xf_naive_div2(elemCount, dst);
    }
    else
    {
        xf_clear(elemCount, dst);
    }
}

internal void
xf_tanh(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumE[elemCount];

    u32 accumR[elemCount];
    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumR);
    xf_make_positive(elemCount, accumR);

    if (xf_compare(elemCount, accumR, gXF_One) >= 0)
    {
        /* This subroutine is used by the exponential function routine.
         * tanh(x) = (exp(x) - exp(-x)) / (exp(x) + exp(-x))
         * Note qexp() calls qtanh, but with an argument less than (1 + log 2)/2.
     */
        xf_exp(elemCount, accumR, accumE);               // E = e^R
        xf_div(elemCount, gXF_One, accumE, accumR);   // R = 1 / E (e^-R)
        xf_sub(elemCount, accumE, accumR, dst);     // dst = E - R
        xf_add(elemCount, accumE, accumR, accumE);       // E += R
        xf_div(elemCount, dst, accumE, dst);          // dst /= E
    }
    else
    {
        u32 accumJ[elemCount];
        u32 accumX[elemCount];

        /* Adjust loop count for convergence to working precision.  */
        u32 n = XFLOAT_MAX_BITS(elemCount) / 9 + 1;
        s64 lj = 2 * n + 1;
        xf_from_s64(elemCount, lj, accumJ);

        xf_copy(elemCount, accumJ, accumE);
        xf_mul(elemCount, src, src, accumX);

        /* continued fraction */
        for (u32 i = 0; i < n; ++i)
        {
            xf_div(elemCount, accumX, accumE, accumR);    // R = X / E
            xf_sub(elemCount, accumJ, gXF_Two, accumJ); // J = J - 2
            xf_add(elemCount, accumR, accumJ, accumE);       // E = R + J
        }

        xf_div(elemCount, src, accumE, dst);
    }

    if (sign)
    {
        xf_make_negative(elemCount, dst);
    }
}

internal void
xf_asinh(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumX[elemCount];
    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumX);
    xf_make_positive(elemCount, accumX);

    if (xf_get_exponent(elemCount, accumX) >= (XFLOAT_EXP_ONE - 4))
    {
        if ((s32)(xf_get_exponent(elemCount, accumX) - XFLOAT_EXP_ONE) >= (((s32)XFLOAT_MAX_EXPONENT - XFLOAT_EXP_ONE) / 2))
        {
            xf_log(elemCount, accumX, dst);
            xf_add(elemCount, gXF_Log2, dst, dst);
        }
        else
        {
            xf_mul(elemCount, accumX, accumX, dst);
            xf_add(elemCount, gXF_One, dst, dst);
            xf_square_root(elemCount, dst, dst);
            xf_add(elemCount, accumX, dst, dst);
            xf_log(elemCount, dst, dst);
        }

        if (sign)
        {
            xf_negate(elemCount, dst);
        }
    }
    else
    {
        u32 accumZ[elemCount];
        u32 accumA[elemCount];
        u32 accumB[elemCount];
        u32 accumS[elemCount];
        u32 accumX2[elemCount];

        xf_mul(elemCount, accumX, accumX, accumZ);
        xf_add(elemCount, gXF_One, accumZ, accumA);
        xf_square_root(elemCount, accumA, accumA);

        u32 i = XFLOAT_MAX_BITS(elemCount) / 6;
        s64 n = 2 * i + 1;

        xf_clear(elemCount, accumS);

        do
        {
            s64 j = i * (i - 1);
            xf_from_s64(elemCount, j, accumB);
            xf_mul(elemCount, accumB, accumZ, accumB);
            xf_from_s64(elemCount, n, accumX2);
            xf_add(elemCount, accumS, accumX2, accumX2);
            xf_div(elemCount, accumB, accumX2, accumS);
            n -= 2;
            xf_from_s64(elemCount, n, accumX2);
            xf_add(elemCount, accumS, accumX2, accumX2);
            xf_div(elemCount, accumB, accumX2, accumS);
            n -= 2;
            i -= 2;
        } while (n > 1);

        xf_add(elemCount, gXF_One, accumS, accumS);
        xf_div(elemCount, accumA, accumS, accumA);
        xf_mul(elemCount, accumA, accumX, dst);
        if (sign)
        {
            xf_negate(elemCount, dst);
        }
    }
}

internal void
xf_acosh(u32 elemCount, u32 *src, u32 *dst)
{
    if (xf_compare(elemCount, src, gXF_One) < 0)
    {
        // NOTE(michiel): Domain error
        xf_clear(elemCount, dst);
    }
    else if (xf_get_exponent(elemCount, src) > (XFLOAT_EXP_ONE + XFLOAT_MAX_BITS(elemCount)))
    {
        xf_log(elemCount, src, dst);                   // Y = LOG(X)
        xf_add(elemCount, gXF_Log2, dst, dst);         // Y += LOG(2)  | LOG(2) + LOG(X)
    }
    else
    {
        u32 accum[elemCount];
        xf_mul(elemCount, src, src, accum);       // A = X^2
        xf_sub(elemCount, accum, gXF_One, accum); // A -= 1       | X^2 - 1
        xf_square_root(elemCount, accum, accum);       // A = SQRT(A)  | SQRT(X^2 - 1)
        xf_add(elemCount, src, accum, accum);          // A += X       | X + SQRT(X^2 - 1)
        xf_log(elemCount, accum, dst);                 // Y = LOG(A)   | LOG(X + SQRT(X^2 - 1))
    }
}

internal void
xf_atanh(u32 elemCount, u32 *src, u32 *dst)
{
    u32 accumA[elemCount];
    u32 accumB[elemCount];

    u32 sign = xf_get_sign(elemCount, src);
    xf_copy(elemCount, src, accumA);
    xf_make_positive(elemCount, accumA);                // A = ABS(X)

    if (xf_compare(elemCount, accumA, gXF_One) >= 0)
    {
        // NOTE(michiel): Domain error
        xf_infinite(elemCount, dst);
        dst[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
        dst[XFLOAT_SIGN_EXP_IDX] |= sign;
    }
    else if (((s64)XFLOAT_EXP_ONE - (s64)xf_get_exponent(elemCount, src)) >= (XFLOAT_MAX_BITS(elemCount) / 4))
    {
        // NOTE(michiel): X + X^3/3 + X^5/5
        xf_mul(elemCount, accumA, accumA, accumB);  // B = A^2     | |X|^2
        xf_mul(elemCount, accumA, accumB, dst);     // Y = A^3     | |X|^3

        xf_mul(elemCount, accumB, dst, accumB);     // B = A^5     | |X|^5
        xf_div(elemCount, dst, gXF_Three, dst);       // Y = A^3/3   | |X|^3 / 3

        xf_div(elemCount, accumB, gXF_Five, accumB);  // B = A^5/5   | |X|^5 / 5
        xf_add(elemCount, accumB, dst, dst);             // Y += B      | |X|^3 / 3 + |X|^5 / 5
        xf_add(elemCount, accumA, dst, dst);             // Y += A      | |X| + |X|^3 / 3 + |X|^5 / 5

        dst[XFLOAT_SIGN_EXP_IDX] &= ~XFLOAT_SIGN_MASK;
        dst[XFLOAT_SIGN_EXP_IDX] |= sign;                // Y = SIGN(X)
    }
    else
    {
        // NOTE(michiel): 0.5 * log((1+x)/(1-x))
        xf_add(elemCount, gXF_One, src, accumA);         // A = 1 + X      | 1 + X
        xf_sub(elemCount, gXF_One, src, dst);       // Y = 1 - X      | 1 - X
        xf_div(elemCount, accumA, dst, accumA);       // A /= Y         | (1 + X) / (1 - X)
        xf_log(elemCount, accumA, dst);                  // Y = LOG(A)     | LOG((1 + X) / (1 - X))
        if (xf_get_exponent(elemCount, dst))
        {
            xf_naive_div2(elemCount, dst);               // Y /= 2         | 0.5*LOG((1 + X) / (1 - X))
        }
        else
        {
            xf_clear(elemCount, dst);
        }
    }
}

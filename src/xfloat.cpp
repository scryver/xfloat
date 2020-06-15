//
// NOTE(michiel): Shifts
//     None of these functions adjust the exponent.
//

internal void
xf_shift_up16(u32 elemCount, u32 *x)
{
    x += elemCount - 1;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x >> 16;
        *x <<= 16;
        *x-- |= oldBits;
        oldBits = newBits;
    }
}

internal void
xf_shift_down16(u32 elemCount, u32 *x)
{
    x += XFLOAT_MANTISSA_IDX;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x << 16;
        *x >>= 16;
        *x++ |= oldBits;
        oldBits = newBits;
    }
}

internal void
xf_shift_up8(u32 elemCount, u32 *x)
{
    x += elemCount - 1;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x >> 24;
        *x <<= 8;
        *x-- |= oldBits;
        oldBits = newBits;
    }
}
internal void
xf_shift_down8(u32 elemCount, u32 *x)
{
    x += XFLOAT_MANTISSA_IDX;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x << 24;
        *x >>= 8;
        *x++ |= oldBits;
        oldBits = newBits;
    }
}

internal void
xf_shift_up4(u32 elemCount, u32 *x)
{
    x += elemCount - 1;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x >> 28;
        *x <<= 4;
        *x-- |= oldBits;
        oldBits = newBits;
    }
}
internal void
xf_shift_down4(u32 elemCount, u32 *x)
{
    x += XFLOAT_MANTISSA_IDX;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x << 28;
        *x >>= 4;
        *x++ |= oldBits;
        oldBits = newBits;
    }
}

internal void
xf_shift_up2(u32 elemCount, u32 *x)
{
    x += elemCount - 1;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x >> 30;
        *x <<= 2;
        *x-- |= oldBits;
        oldBits = newBits;
    }
}
internal void
xf_shift_down2(u32 elemCount, u32 *x)
{
    x += XFLOAT_MANTISSA_IDX;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x << 30;
        *x >>= 2;
        *x++ |= oldBits;
        oldBits = newBits;
    }
}

internal void
xf_shift_up1(u32 elemCount, u32 *x)
{
    x += elemCount - 1;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x >> 31;
        *x <<= 1;
        *x-- |= oldBits;
        oldBits = newBits;
    }
}

internal void
xf_shift_down1(u32 elemCount, u32 *x)
{
    x += XFLOAT_MANTISSA_IDX;
    u32 oldBits = 0;
    for (u32 index = 0; index < elemCount - XFLOAT_MANTISSA_IDX; ++index)
    {
        u32 newBits = *x << 31;
        *x >>= 1;
        *x++ |= oldBits;
        oldBits = newBits;
    }
}

internal s32
xf_shift(u32 elemCount, u32 *x, s32 *shiftCount)
{
    u32 *p;
#if XFLOAT_STICKY_BIT
    s32 lost;
#endif

    s32 shifts = *shiftCount;

#if XFLOAT_STICKY_BIT
    lost = 0;
#endif

    if (shifts != 0)
    {
        if (shifts < 0)
        {
            p = x + elemCount - 1;
            shifts = -shifts;
            while (shifts >= 16)
            {
#if XFLOAT_STICKY_BIT
                lost |= *p;
#endif
                xf_shift_down16(elemCount, x);
                shifts -= 16;
            }

            if (shifts >= 8)
            {
#if XFLOAT_STICKY_BIT
                lost |= *p & 0xff;
#endif
                xf_shift_down8(elemCount, x);
                shifts -= 8;
            }

            if (shifts >= 4)
            {
#if XFLOAT_STICKY_BIT
                lost |= *p & 0xf;
#endif
                xf_shift_down4(elemCount, x);
                shifts -= 4;
            }

            if (shifts >= 2)
            {
#if XFLOAT_STICKY_BIT
                lost |= *p & 0x3;
#endif
                xf_shift_down2(elemCount, x);
                shifts -= 2;
            }

            // TODO(michiel): Can be an if, I guess...
            while (shifts > 0)
            {
#if XFLOAT_STICKY_BIT
                lost |= *p & 1;
#endif
                xf_shift_down1(elemCount, x);
                shifts -= 1;
            }
        }
        else
        {
            while (shifts >= 16)
            {
                xf_shift_up16(elemCount, x);
                shifts -= 16;
            }

            if (shifts >= 8)
            {
                xf_shift_up8(elemCount, x);
                shifts -= 8;
            }

            if (shifts >= 4)
            {
                xf_shift_up4(elemCount, x);
                shifts -= 4;
            }

            if (shifts >= 2)
            {
                xf_shift_up2(elemCount, x);
                shifts -= 2;
            }

            while (shifts > 0)
            {
                xf_shift_up1(elemCount, x);
                shifts -= 1;
            }
        }
        *shiftCount = shifts;
    }

#if XFLOAT_STICKY_BIT
    return lost;
#else
    return 0;
#endif
}

//
// NOTE(michiel): Core mantissa functions
//   These (almost) all expect an extra word at the lowest precision, so be warned.
//   For normal use see the xf_add, xf_sub, xf_div and xf_mul implementations.
//

internal void
xf_add_mantissa(u32 elemCount, u32 *x, u32 *y)
{
    // NOTE(michiel): y += x
    x += elemCount - 1;
    y += elemCount - 1;

    u64 summer = 0;
    for (u32 loop = 0; loop < elemCount - XFLOAT_MANTISSA_IDX; ++loop)
    {
        summer = (u64)(*y) + (u64)(*x--) + summer;
        *y-- = (u32)summer;
        summer = (summer >> 32) & 0x1;
    }
}

internal void
xf_subtract_mantissa(u32 elemCount, u32 *x, u32 *y)
{
    // NOTE(michiel): y -= x
    x += elemCount - 1;
    y += elemCount - 1;

    u64 subber = 0;
    for (u32 loop = 0; loop < elemCount - XFLOAT_MANTISSA_IDX; ++loop)
    {
        subber = (u64)(*y) - (u64)(*x--) - subber;
        *y-- = (u32)subber;
        subber = (subber >> 32) & 0x1;
    }
}

internal s32
xf_compare_mantissa(u32 elemCount, u32 *x, u32 *y)
{
    // NOTE(michiel): {-1: x < y, 0: x = y, 1: x > y}
    s32 result = 0;
    x += XFLOAT_MANTISSA_IDX;
    y += XFLOAT_MANTISSA_IDX;
    for (u32 i = 0; i < elemCount - XFLOAT_MANTISSA_IDX; ++i)
    {
        u32 valX = *x++;
        u32 valY = *y++;
        if (valX > valY)
        {
            result = 1;
            break;
        }
        else if (valX < valY)
        {
            result = -1;
            break;
        }
    }

    return result;
}

internal void
xf_muldiv_normalize(u32 elemCount, u32 *x)
{
    // NOTE(michiel): The expected elemCount for this function to round
    // properly, is your normal elemCount + 1 at least. For normal use cases,
    // you won't need this however.
    u32 roundBit[elemCount];
    xf_clear(elemCount, roundBit);
    roundBit[elemCount - 2] = 1;

    s32 exponent = (s32)xf_get_exponent(elemCount, x);

    for (u32 i = 0; i < 3; ++i)
    {
        if (x[XFLOAT_MANTISSA_IDX] == 0) {
            break;
        }

        xf_shift_down1(elemCount, x);
        if (exponent < XFLOAT_MAX_EXPONENT)
        {
            ++exponent;
        }
        else
        {
            exponent = XFLOAT_MAX_EXPONENT;
        }
    }

    for (u32 i = 0; i < 3; ++i)
    {
        if (x[XFLOAT_MANTISSA_IDX + 1] & XFLOAT_HIGH_BIT) {
            break;
        }

        /* Prevent exponent underflow.
        Rounding may be incorrect when this happens.  */
        // TODO(michiel): Maybe clear out to 0 when this happens...
        if (exponent >= 1)
        {
            xf_shift_up1(elemCount, x);
            --exponent;
        }
    }

    if (x[elemCount - 1] & XFLOAT_HIGH_BIT)
    {
        xf_add_mantissa(elemCount, roundBit, x);
    }

    if (x[XFLOAT_MANTISSA_IDX])
    {
        xf_shift_down1(elemCount, x);
        if (exponent < XFLOAT_MAX_EXPONENT)
        {
            ++exponent;
        }
        else
        {
            exponent = XFLOAT_MAX_EXPONENT;
        }
    }
    xf_set_exponent(elemCount, x, exponent);
    x[elemCount - 1] = 0;
}

internal void
xf_multiply_precision(u32 elemCount, u32 precision, u32 *a, u32 *b, u32 *c)
{
    i_expect(a != c);
    i_expect(b != c);
    i_expect(precision + XFLOAT_MANTISSA_IDX + 1 < elemCount);

    u32 k = precision + XFLOAT_MANTISSA_IDX;
    u32 *zeroer = c + XFLOAT_MANTISSA_IDX;
    do {
        *zeroer++ = 0;
    } while (--k);

    u32 *r = c + precision + XFLOAT_MANTISSA_IDX + 1;
    for (u32 idx = precision + XFLOAT_MANTISSA_IDX; idx >= XFLOAT_MANTISSA_IDX + 1; --idx)
    {
        u32 *q = b + XFLOAT_MANTISSA_IDX + 1;
        u32 *p = a + idx;
        for (u32 innerIdx = idx; innerIdx >= XFLOAT_MANTISSA_IDX + 1; --innerIdx)
        {
            u32 valP = *p--;
            u32 valQ = *q++;
            if (valP && valQ)
            {
                // TODO(michiel): Could remove this if statement and just always multiply
                u64 lp = (u64)valP * (u64)valQ;
                u64 u  = (u64)(*r) + (u32)lp;
                *r = u;
                u = (u64)(*(r - 1)) + (lp >> 32) + (u >> 32);
                *(r - 1) = u;
                *(r - 2) += u >> 32;
            }
        }
        --r;
    }
}

internal void
xf_square_precision(u32 elemCount, u32 precision, u32 *a, u32 *b)
{
    i_expect(a != b);
    i_expect(precision + XFLOAT_MANTISSA_IDX + 1 < elemCount);

    u32 k = precision + XFLOAT_MANTISSA_IDX;
    u32 *zeroer = b + XFLOAT_MANTISSA_IDX;

    do {
        *zeroer++ = 0;
    } while (--k);

    u32 *r = b + precision + XFLOAT_MANTISSA_IDX + 1;
    for (u32 idx = precision + XFLOAT_MANTISSA_IDX; idx >= XFLOAT_MANTISSA_IDX + 1; --idx)
    {
        u32 *q = a + XFLOAT_MANTISSA_IDX + 1;
        u32 *p = a + idx;
        while (p >= q)
        {
            if ((*p == 0) || (*q == 0))
            {
                --p;
                ++q;
            }
            else
            {
                u64 lp = (u64)(*p) * (*q);
                if (p != q)
                {
                    if (lp & ((u64)XFLOAT_HIGH_BIT << 32))
                    {
                        *(r - 2) += 1;
                    }
                    lp <<= 1;
                }
                --p;
                ++q;
                u64 u = (u64)(*r) + (u32)lp;
                *r = u;
                u = (u64)(*(r - 1)) + (lp >> 32) + (u >> 32);
                *(r - 1) = u;
                *(r - 2) += u >> 32;
            }
        }
        --r;
    }

    xf_shift_up1(elemCount, b);
}

internal void
xf_multiply_single(u32 elemCount, u32 *a, u32 *b)
{
    // NOTE(michiel): b *= a, where a has only 1 significand word
    u32 accum[elemCount + 1];
    xf_clear(elemCount + 1, accum);

    accum[XFLOAT_SIGN_EXP_IDX] = b[XFLOAT_SIGN_EXP_IDX];

    u32 *r = accum + elemCount;
    u64 y = a[XFLOAT_MANTISSA_IDX + 1];
    u32 *p = b + elemCount - 1;

    for (u32 index = elemCount - 1; index >= XFLOAT_MANTISSA_IDX + 1; --index)
    {
        u32 valP = *p--;
        if (valP)
        {
            u64 lp = (u64)valP * y;
            u64 la = (u64)(*r) + (u32)lp;
            *r = la;
            la = (u64)(*(r - 1)) + (lp >> 32) + (la >> 32);
            *(r - 1) = la;
            *(r - 2) += la >> 32;
        }
        --r;
    }
    xf_muldiv_normalize(elemCount + 1, accum);
    xf_copy(elemCount, accum, b);
}

internal void
xf_multiply_mantissa(u32 elemCount, u32 *a, u32 *b)
{
    // NOTE(michiel): b *= a
    u32 accum[elemCount + 2];
    xf_clear(elemCount + 2, accum);

    accum[XFLOAT_SIGN_EXP_IDX] = b[XFLOAT_SIGN_EXP_IDX];

    // TODO(michiel): Could check for a value with only 1 significand word and use xf_multiply_single

    u32 *r = accum + elemCount + 1;
    for (u32 k = elemCount; k >= XFLOAT_MANTISSA_IDX + 1; --k)
    {
        u32 m = k;
        u32 o = XFLOAT_MANTISSA_IDX + 1;
        if (k == elemCount)
        {
            m = elemCount - 1;
            o = XFLOAT_MANTISSA_IDX + 2;
        }
        u32 *q = b + o;
        u32 *p = a + m;

        for (u32 i = m; i >= o; --i)
        {
            u32 valP = *p--;
            u32 valQ = *q++;
            if (valP && valQ)
            {
                u64 lp = (u64)valP * (u64)valQ;
                u64 la = (u64)(*r) + (u32)lp;
                *r = la;
                la = (u64)(*(r - 1)) + (lp >> 32) + (la >> 32);
                *(r - 1) = la;
                *(r - 2) += la >> 32;
            }
        }
        --r;
    }
    xf_muldiv_normalize(elemCount + 1, accum);
    xf_copy(elemCount, accum, b);
}

internal void
xf_divide_mantissa(u32 elemCount, u32 *a, u32 *b)
{
    i_expect(elemCount > (XFLOAT_MANTISSA_IDX + 2));
    // NOTE(michiel): b /= a
    u32 sqr[elemCount + 2];
    u32 prod[elemCount + 2];
    u32 quot[elemCount + 2];

    /* Test if denominator has only 32 bits of significance. */
    u32 *p = a + XFLOAT_MANTISSA_IDX + 2;
    u32 elemIdx = elemCount - (XFLOAT_MANTISSA_IDX + 2);
    b32 doLongDiv = false;
    do
    {
        if (*p++ != 0)
        {
            doLongDiv = true;
            break;
        }
    }
    while (--elemIdx);

    if (!doLongDiv)
    {
        /* Do single precision divides if so. */
        xf_copy_extend(elemCount, b, prod);
        prod[elemCount + 1] = 0;
        xf_shift_down2(elemCount + 2, prod);
        u32 d = a[XFLOAT_MANTISSA_IDX + 1];
        u64 u = ((u64)prod[XFLOAT_MANTISSA_IDX + 1] << 32) | prod[XFLOAT_MANTISSA_IDX + 2];
        for (u32 idx = XFLOAT_MANTISSA_IDX + 1; idx < elemCount; ++idx)
        {
            u32 qu = u / d;
            prod[idx] = qu;
            u = ((u - (u64)d * qu) << 32) | prod[idx + 2];
        }
        prod[elemCount] = u / d;
    }
    else
    {
        /* Slower procedure is required */
        xf_clear(elemCount + 2, quot);
        xf_clear(elemCount + 2, prod);
        xf_clear(elemCount + 2, sqr);
        quot[XFLOAT_MANTISSA_IDX + 1] = ((u64)0x4000000000000000ULL) / a[XFLOAT_MANTISSA_IDX + 1];

        u32 precision = 1;
        while (precision < (elemCount - XFLOAT_MANTISSA_IDX))
        {
            precision *= 2;
            if (precision > (elemCount - XFLOAT_MANTISSA_IDX)) {
                precision = elemCount - XFLOAT_MANTISSA_IDX;
            }

            xf_square_precision(elemCount + 2, precision, quot, sqr);
            xf_multiply_precision(elemCount + 2, precision, a, sqr, prod);
            xf_subtract_mantissa(elemCount + 2, prod, quot);
            xf_shift_up1(elemCount + 2, quot);
        }

        xf_multiply_precision(elemCount + 2, elemCount - XFLOAT_MANTISSA_IDX, quot, b, prod);
        prod[XFLOAT_SIGN_EXP_IDX] = b[XFLOAT_SIGN_EXP_IDX];
    }

    xf_muldiv_normalize(elemCount + 1, prod);
    xf_copy(elemCount, prod, b);
}

//
// NOTE(michiel): Normalization
//

internal s32
xf_normalize_mantissa(u32 elemCount, u32 *x, s32 *shiftCount)
{
    // NOTE(michiel): Returns amounts of shifts needed to normalize in _shiftCount_, this
    //   info can be used to readjust the exponent.
    s32 result = 0;

    s32 count = 0;
    u32 *p = x + XFLOAT_MANTISSA_IDX;
    if (*p != 0)
    {
#if 0
        if (*p & 0xFFFF8000) {
            xf_shift_down16(elemCount, x);
            count -= 16;
        }
        if (*p & 0xFFFFFF80) {
            xf_shift_down8(elemCount, x);
            count -= 8;
        }
        if (*p & 0xFFFFFFF8) {
            xf_shift_down4(elemCount, x);
            count -= 4;
        }
        if (*p & 0xFFFFFFFE) {
            xf_shift_down2(elemCount, x);
            count -= 2;
        }
        if (*p) {
            xf_shift_down1(elemCount, x);
            count -= 1;
        }
#else
        while (*p & 0xFFFFFF00)
        {
            xf_shift_down8(elemCount, x);
            count -= 8;
        }
        while (*p != 0)
        {
            xf_shift_down1(elemCount, x);
            --count;
        }
#endif
    }
    else
    {
        ++p;
        if (*p & XFLOAT_HIGH_BIT)
        {
            // NOTE(michiel): Nothing to do
        }
        else
        {
            while (*p == 0)
            {
                // TODO(michiel): Move to 32 bit shifts
                xf_shift_up16(elemCount, x);
                xf_shift_up16(elemCount, x);
                count += 32;
                if (count > XFLOAT_MAX_BITS(elemCount - 1))
                {
                    result = 1;
                    break;
                }
            }

            if (!result)
            {
#if 0
                if ((*p & 0xFFFF0000) == 0) {
                    xf_shift_up16(elemCount, x);
                    count += 16;
                }
                if ((*p & 0xFF000000) == 0) {
                    xf_shift_up8(elemCount, x);
                    count += 8;
                }
                if ((*p & 0xF0000000) == 0) {
                    xf_shift_up4(elemCount, x);
                    count += 4;
                }
                if ((*p & 0xC0000000) == 0) {
                    xf_shift_up2(elemCount, x);
                    count += 2;
                }
                if ((*p & 0x80000000) == 0) {
                    xf_shift_up1(elemCount, x);
                    count += 1;
                }
#else
                while ((*p & 0xFF000000) == 0)
                {
                    xf_shift_up8(elemCount, x);
                    count += 8;
                }
                while ((*p & XFLOAT_HIGH_BIT) == 0)
                {
                    xf_shift_up1(elemCount, x);
                    ++count;
                }
#endif
            }
        }
    }
    *shiftCount = count;

    return result;
}

//
// NOTE(michiel): Base math on total floats, internal functions
//

internal void
xf_add_internal(u32 elemCount, u32 *a, u32 *b, u32 *c, b32 doSub = false)
{
    u32 accumBuf1[elemCount + 1];
    u32 accumBuf2[elemCount + 1];
    u32 *accum1 = accumBuf1;
    u32 *accum2 = accumBuf2;

    xf_copy_extend(elemCount, a, accum1);
    xf_copy_extend(elemCount, b, accum2);

    if (doSub)
    {
        xf_negate(elemCount + 1, accum2);
    }

    /* compare exponents */
    s64 lt = (s64)xf_get_exponent(elemCount, accum2) - (s64)xf_get_exponent(elemCount, accum1);
    if (lt > 0)
    {
        /* put the larger number in accum1 */
        u32 *temp = accum1;
        accum1 = accum2;
        accum2 = temp;
        lt = -lt;
    }

    s32 shiftCount = lt;
#if XFLOAT_STICKY_BIT
    s32 lost = 0;
#endif

    u32 doneFlags = 0;
    if (lt != 0)
    {
        if (lt < -(s32)XFLOAT_MAX_BITS(elemCount) - 1)
        {
            /* answer same as larger addend */
            doneFlags |= XFloatCompletion_Copy;
        }
        else
        {
#if XFLOAT_STICKY_BIT
            lost = xf_shift(elemCount + 1, accum2, &shiftCount); /* shift the smaller number down */
#else
            xf_shift(elemCount + 1, accum2, &shiftCount); /* shift the smaller number down */
#endif
        }
    }
    else
    {
        /* exponents were the same, so must compare mantissae */
        s32 i = xf_compare_mantissa(elemCount, accum2, accum1);
        if (i == 0)
        {
            /* the numbers are identical */
            if (xf_get_sign(elemCount, accum2) != xf_get_sign(elemCount, accum1))
            {
                /* if different signs, result is zero */
                doneFlags |= XFloatCompletion_Underflow;
            }
            else if (xf_get_exponent(elemCount, accum2) == 0)
            {
                /* if exponents zero, result is zero */
                doneFlags |= XFloatCompletion_Underflow;
            }
            else if (xf_get_exponent(elemCount, accum2) >= XFLOAT_MAX_EXPONENT)
            {
                /* if same sign, result is double */
                xf_clear(elemCount, c);
                if (xf_get_sign(elemCount, accum2) != 0)
                {
                    xf_flip_sign(elemCount, c);
                }
                doneFlags |= XFloatCompletion_Overflow;
            }
            else
            {
                u32 exponent = xf_get_exponent(elemCount, accum1);
                xf_set_exponent(elemCount, accum1, exponent + 1);
                doneFlags |= XFloatCompletion_Copy;
            }
        }
        else if (i > 0)
        {
            /* put the larger number in accum1 */
            u32 *temp = accum1;
            accum1 = accum2;
            accum2 = temp;
        }
    }

    if (!doneFlags)
    {
        if (xf_get_sign(elemCount, accum2) == xf_get_sign(elemCount, accum1))
        {
            xf_add_mantissa(elemCount + 1, accum2, accum1);
            doSub = false;
        }
        else
        {
            xf_subtract_mantissa(elemCount + 1, accum2, accum1);
            doSub = true;
        }

        if (xf_normalize_mantissa(elemCount + 1, accum1, &shiftCount))
        {
            doneFlags |= XFloatCompletion_Underflow;
        }
        else
        {
            lt = (s64)xf_get_exponent(elemCount, accum1) - shiftCount;
            if (lt > (s64)XFLOAT_MAX_EXPONENT)
            {
                doneFlags |= XFloatCompletion_Overflow;
            }
            else if (lt < 0)
            {
                doneFlags |= XFloatCompletion_Underflow;
            }
            else
            {
                xf_set_exponent(elemCount, accum1, lt);

                /* round off */
                u32 i = accum1[elemCount];

                if (i & XFLOAT_HIGH_BIT)
                {
#if XFLOAT_STICKY_BIT
                    if (i == XFLOAT_HIGH_BIT)
                    {
                        if (lost == 0)
                        {
                            /* Critical case, round to even */
                            if ((accum1[elemCount - 1] & 1) == 0)
                            {
                                doneFlags |= XFloatCompletion_Copy;
                            }
                        }
                        else if (doSub != 0)
                        {
                            doneFlags |= XFloatCompletion_Copy;
                        }
                    }
#else
                    if (doSub != 0)
                    {
                        doneFlags |= XFloatCompletion_Copy;
                    }
#endif
                    if (!doneFlags)
                    {
                        xf_clear(elemCount + 1, accum2);
                        accum2[elemCount - 1] = 1;
                        xf_add_mantissa(elemCount + 1, accum2, accum1);
                        xf_normalize_mantissa(elemCount + 1, accum1, &shiftCount);
                        if (shiftCount)
                        {
                            lt = (s64)xf_get_exponent(elemCount, accum1) - shiftCount;
                            if (lt > (s64)XFLOAT_MAX_EXPONENT)
                            {
                                doneFlags |= XFloatCompletion_Overflow;
                            }
                            else
                            {
                                xf_set_exponent(elemCount, accum1, lt);
                            }
                        }
                    }
                }
            }
        }
    }

    if (doneFlags & XFloatCompletion_Underflow)
    {
        xf_clear(elemCount, c);
    }
    else if (doneFlags & XFloatCompletion_Overflow)
    {
        xf_infinite(elemCount, c);
    }
    else
    {
        xf_copy(elemCount, accum1, c);
    }
}

internal void
xf_multiply_int(u32 elemCount, u32 *a, u32 *b, u32 *c)
{
    u32 accum[elemCount + 1];

    if ((xf_get_exponent(elemCount, a) == 0) ||
        (xf_get_exponent(elemCount, b) == 0))
    {
        xf_clear(elemCount, c);
    }
    else
    {
        xf_copy(elemCount, b, accum);
        xf_multiply_single(elemCount, a, accum);

        /* calculate sign of product */
        xf_multiply_sign(elemCount, a, b, accum);

        /* calculate exponent */
        s64 lt = xf_unbiased_exponent(elemCount, accum) + xf_unbiased_exponent(elemCount, a);
        if (lt <= (s64)XFLOAT_MAX_EXPONENT)
        {
            xf_set_exponent(elemCount, accum, lt);
            accum[elemCount] = 0;
            s32 shiftCount;
            if (xf_normalize_mantissa(elemCount + 1, accum, &shiftCount))
            {
                xf_clear(elemCount, c);
            }
            else
            {
                lt = lt - shiftCount + XFLOAT_EXP_BIAS;
                if (lt > (s64)XFLOAT_MAX_EXPONENT)
                {
                    xf_infinite(elemCount, c);
                }
                else if (lt < 0)
                {
                    xf_clear(elemCount, c);
                }
                else
                {
                    xf_set_exponent(elemCount, accum, lt);
                    xf_copy(elemCount, accum, c);
                }
            }
        }
        else
        {
            xf_infinite(elemCount, c);
        }
    }
}

//
// NOTE(michiel): Base math on total floats, external functions
//

internal void
xf_minimum(u32 elemCount, u32 *src1, u32 *src2, u32 *dst)
{
    if (xf_compare(elemCount, src1, src2) < 0)
    {
        xf_copy(elemCount, src1, dst);
    }
    else
    {
        xf_copy(elemCount, src2, dst);
    }
}

internal void
xf_maximum(u32 elemCount, u32 *src1, u32 *src2, u32 *dst)
{
    if (xf_compare(elemCount, src1, src2) > 0)
    {
        xf_copy(elemCount, src1, dst);
    }
    else
    {
        xf_copy(elemCount, src2, dst);
    }
}

internal void
xf_add(u32 elemCount, u32 *src1, u32 *src2, u32 *dst)
{
    // NOTE(michiel): dst = src1 + src2
    xf_add_internal(elemCount, src1, src2, dst, false);
}

internal void
xf_sub(u32 elemCount, u32 *src1, u32 *src2, u32 *dst)
{
    // NOTE(michiel): dst = src1 - src2
    xf_add_internal(elemCount, src1, src2, dst, true);
}

internal void
xf_mul(u32 elemCount, u32 *src1, u32 *src2, u32 *dst)
{
    // NOTE(michiel): dst = src1 * src2
    i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);
    if ((xf_get_exponent(elemCount, src1) == 0) ||
        (xf_get_exponent(elemCount, src2) == 0))
    {
        xf_clear(elemCount, dst);
    }
    else
    {
        s64 lt;
        u32 accum[elemCount + 1];

        b32 src1IsSingle = false;
        if (src1[XFLOAT_MANTISSA_IDX + 2] == 0)
        {
            src1IsSingle = true;
            u32 *p = src1 + XFLOAT_MANTISSA_IDX + 3;
            for (u32 idx = XFLOAT_MANTISSA_IDX + 3; idx < elemCount; ++idx)
            {
                if (*p++ != 0)
                {
                    src1IsSingle = false;
                    break;
                }
            }

            if (src1IsSingle)
            {
                xf_copy(elemCount, src2, accum);
                xf_multiply_single(elemCount, src1, accum);
                lt = xf_unbiased_exponent(elemCount, src1) + xf_unbiased_exponent(elemCount, accum);
            }
        }

        if (!src1IsSingle)
        {
            b32 src2IsSingle = false;
            if (src2[XFLOAT_MANTISSA_IDX + 2] == 0)
            {
                src2IsSingle = true;
                u32 *p = src2 + XFLOAT_MANTISSA_IDX + 3;
                for (u32 idx = XFLOAT_MANTISSA_IDX + 3; idx < elemCount; ++idx)
                {
                    if (*p++ != 0)
                    {
                        src2IsSingle = false;
                        break;
                    }
                }

                if (src2IsSingle)
                {
                    xf_copy(elemCount, src1, accum);
                    xf_multiply_single(elemCount, src2, accum);
                    lt = xf_unbiased_exponent(elemCount, src2) + xf_unbiased_exponent(elemCount, accum);
                }
            }

            if (!src2IsSingle)
            {
                xf_copy(elemCount, src1, accum);
                xf_multiply_mantissa(elemCount, src2, accum);
                lt = xf_unbiased_exponent(elemCount, src2) + xf_unbiased_exponent(elemCount, accum);
            }
        }

        xf_multiply_sign(elemCount, src1, src2, accum);

        if (lt > (s64)XFLOAT_MAX_EXPONENT)
        {
            xf_infinite(elemCount, dst);
        }
        else
        {
            xf_set_exponent(elemCount, accum, lt);
            accum[elemCount] = 0;
            s32 shiftCount;
            if (xf_normalize_mantissa(elemCount + 1, accum, &shiftCount))
            {
                xf_clear(elemCount, dst);
            }
            else
            {
                lt = lt - shiftCount + XFLOAT_EXP_BIAS;
                if (lt > (s64)XFLOAT_MAX_EXPONENT)
                {
                    xf_infinite(elemCount, dst);
                }
                else if (lt <= 0)
                {
                    xf_clear(elemCount, dst);
                }
                else
                {
                    xf_set_exponent(elemCount, accum, lt);
                    xf_copy(elemCount, accum, dst);
                }
            }
        }
    }
}

internal void
xf_div(u32 elemCount, u32 *src1, u32 *src2, u32 *dst)
{
    // NOTE(michiel): dst = src1 / src2
    u32 accum[elemCount + 1];

    if (xf_get_exponent(elemCount, src1) == 0)
    {
        /* numerator is zero */
        xf_clear(elemCount, dst);
    }
    else
    {
        if (xf_get_exponent(elemCount, src2) == 0)
        {   /* divide by zero */
            xf_infinite(elemCount, dst);
        }
        else
        {
            xf_copy_extend(elemCount, src1, accum);

            /* Avoid exponent underflow in mdnorm.  */
            s64 lt = (s64)xf_get_exponent(elemCount, accum);
            xf_set_exponent(elemCount, accum, 4);

            xf_divide_mantissa(elemCount, src2, accum);

            xf_multiply_sign(elemCount, src1, src2, accum);

            /* calculate exponent */
            lt = lt + (s64)xf_get_exponent(elemCount, accum) - 4L - (s64)xf_get_exponent(elemCount, src2);
            xf_set_exponent(elemCount, accum, lt);
            accum[elemCount] = 0;

            s32 shiftCount;
            xf_normalize_mantissa(elemCount + 1, accum, &shiftCount);

            lt = lt - shiftCount + XFLOAT_EXP_ONE + 1;
            if (lt > (s64)XFLOAT_MAX_EXPONENT)
            {
                xf_infinite(elemCount, dst);
            }
            else if (lt <= 0)
            {
                xf_clear(elemCount, dst);
            }
            else
            {
                xf_set_exponent(elemCount, accum, lt);
                xf_copy(elemCount, accum, dst);
            }
        }
    }
}

internal s64
xf_integer_fraction(u32 elemCount, u32 *src, u32 *fraction)
{
    // TODO(michiel): Make the integer part use all 64 bits!

    // NOTE(michiel): Returns the integer and puts the fraction in to _fraction_.
    u32 accum[elemCount + 1];
    s64 result = 0;

    xf_copy_extend(elemCount, src, accum);
    s32 shiftCount = xf_unbiased_exponent(elemCount, accum);

    if (shiftCount <= 0)
    {
        xf_copy(elemCount, accum, fraction);
    }
    else
    {
        if (shiftCount > 31)
        {
            result = 0x7FFFFFFF;
            xf_shift(elemCount + 1, accum, &shiftCount);
        }
        else
        {
            xf_shift(elemCount + 1, accum, &shiftCount);
            result = accum[XFLOAT_MANTISSA_IDX];
        }

        if (xf_get_sign(elemCount, src))
        {
            result = -result;
        }
        xf_make_positive(elemCount, accum);
        xf_set_exponent(elemCount, accum, XFLOAT_EXP_BIAS);
        accum[XFLOAT_MANTISSA_IDX] = 0;
        if (xf_normalize_mantissa(elemCount + 1, accum, &shiftCount))
        {
            xf_clear(elemCount, accum);
        }
        else
        {
            s32 exponent = (s32)xf_get_exponent(elemCount, accum);
            xf_set_exponent(elemCount, accum, exponent - shiftCount);
        }
        xf_copy(elemCount, accum, fraction);
    }

    return result;
}

internal s32
xf_compare(u32 elemCount, u32 *src1, u32 *src2)
{
    // NOTE(michiel): { -1: src1 < q, 0: src1 == q, 1: src1 > q }
    u32 r[elemCount];
    s32 result = 0;

    if ((xf_get_exponent(elemCount, src1) <= (u32)XFLOAT_MAX_BITS(elemCount)) &&
        (xf_get_exponent(elemCount, src2) <= (u32)XFLOAT_MAX_BITS(elemCount)))
    {
        xf_sub(elemCount, src1, src2, r);
        if (xf_get_exponent(elemCount, r) == 0) {
            result = 0;
        } else if (xf_get_sign(elemCount, r) == 0) {
            result = 1;
        } else {
            result = -1;
        }
    }
    else
    {
        if (xf_get_sign(elemCount, src1) != xf_get_sign(elemCount, src2))
        {
            /* the signs are different */
            if (xf_get_sign(elemCount, src1) == 0) {
                result = 1;
            } else {
                result = -1;
            }
        }
        else
        {
            s32 msign;
            /* both are the same sign */
            if (xf_get_sign(elemCount, src1) == 0) {
                msign = 1;
            } else {
                msign = -1;
            }

            u32 *test1 = src1;
            u32 *test2 = src2;
            u32 i = elemCount;
            b32 equality = true;
            do
            {
                if (*test1++ != *test2++)
                {
                    equality = false;
                    break;
                }
            }
            while (--i > 0);

            if (!equality)
            {
                if (*--test1 > *--test2) {
                    result = msign;  /* p is bigger */
                } else {
                    result = -msign; /* p is littler */
                }
            }
        }
    }

    return result;
}

internal void
xf_from_f32(u32 elemCount, f32 f, u32 *x)
{
    i_expect(elemCount > XFLOAT_MANTISSA_IDX + 1);

    u32 u = *(u32 *)&f;
    u32 sign = u & F32_SIGN_MASK;
    u32 exponent = u & F32_EXP_MASK;
    u32 mantissa = u & F32_FRAC_MASK;

    xf_clear(elemCount, x);
    x[XFLOAT_SIGN_EXP_IDX] |= sign ? XFLOAT_SIGN_MASK : 0;

    if (exponent == F32_EXP_MASK)
    {
        xf_infinite(elemCount, x);
    }
    else if (exponent || mantissa)
    {
        b32 denormalized = exponent == 0;

        u32 exp = exponent >> 23;
        exp += XFLOAT_EXP_ONE - 127;
        xf_set_exponent(elemCount, x, exp);

        x[XFLOAT_MANTISSA_IDX + 1] = (denormalized ? 0 : 0x80000000) | (mantissa << 8);

        if (denormalized)
        {
            s32 shiftCount;
            xf_normalize_mantissa(elemCount, x, &shiftCount);
            if (shiftCount > XFLOAT_MAX_BITS(elemCount))
            {
                xf_clear(elemCount, x);
            }
            else
            {
                s64 exponent = (s64)xf_get_exponent(elemCount, x);
                xf_set_exponent(elemCount, x, exponent - (shiftCount - 1));
            }
        }
    }
}

internal void
xf_from_f64(u32 elemCount, f64 f, u32 *x)
{
    i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);

    u64 u = *(u64 *)&f;
    u64 sign = u & F64_SIGN_MASK;
    u64 exponent = u & F64_EXP_MASK;
    u64 mantissa = u & F64_FRAC_MASK;

    xf_clear(elemCount, x);
    x[XFLOAT_SIGN_EXP_IDX] |= sign ? XFLOAT_SIGN_MASK : 0;

    if (exponent == F64_EXP_MASK)
    {
        xf_infinite(elemCount, x);
    }
    else if (exponent || mantissa)
    {
        b32 denormalized = exponent == 0;

        u32 exp = (u32)(exponent >> 52);
        exp += XFLOAT_EXP_ONE - 1023;
        xf_set_exponent(elemCount, x, exp);

        x[XFLOAT_MANTISSA_IDX + 1] = (denormalized ? 0 : 0x80000000) | (u32)(mantissa >> 20);
        x[XFLOAT_MANTISSA_IDX + 2] = (u32)(mantissa << 12);

        if (denormalized)
        {
            s32 shiftCount;
            xf_normalize_mantissa(elemCount, x, &shiftCount);
            if (shiftCount > XFLOAT_MAX_BITS(elemCount))
            {
                xf_clear(elemCount, x);
            }
            else
            {
                s64 exponent = (s64)xf_get_exponent(elemCount, x);
                xf_set_exponent(elemCount, x, exponent - shiftCount);
            }
        }
    }
}

internal void
xf_from_s32(u32 elemCount, s32 s, u32 *x)
{
    i_expect(elemCount > XFLOAT_MANTISSA_IDX + 1);

    xf_clear(elemCount, x);

    if (s)
    {
        if (s < 0)
        {
            xf_make_negative(elemCount, x);
            s = -s; // NOTE(michiel): We account for overflow of the most negative number (this will stay the same)
        }

        BitScanResult highBit = find_most_significant_set_bit(s);
        i_expect(highBit.found); // NOTE(michiel): Otherwise the check on s shouldn't be succesful

        s32 shiftCount = 31 - highBit.index;

        xf_set_exponent(elemCount, x, XFLOAT_EXP_ONE + highBit.index);
        x[XFLOAT_MANTISSA_IDX + 1] = s << shiftCount;
    }
}

internal void
xf_from_s64(u32 elemCount, s64 s, u32 *x)
{
    i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);

    xf_clear(elemCount, x);

    u32 *src = (u32 *)&s;
    if (((src[1] == 0) && !(src[0] & XFLOAT_HIGH_BIT)) ||
        ((src[1] == U32_MAX) && (src[0] & XFLOAT_HIGH_BIT)))
    {
        xf_from_s32(elemCount, src[0], x);
    }
    else
    {
        if (s < 0)
        {
            xf_make_negative(elemCount, x);
            s = -s; // NOTE(michiel): We account for overflow of the most negative number (this will stay the same)
        }

        xf_set_exponent(elemCount, x, XFLOAT_EXP_ONE + 63);
        x[XFLOAT_MANTISSA_IDX + 1] = src[1];
        x[XFLOAT_MANTISSA_IDX + 2] = src[0];

        s32 shiftCount;
        xf_normalize_mantissa(elemCount, x, &shiftCount);
        if (shiftCount > XFLOAT_MAX_BITS(elemCount))
        {
            xf_clear(elemCount, x);
        }
        else
        {
            s64 exponent = (s64)xf_get_exponent(elemCount, x);
            xf_set_exponent(elemCount, x, exponent - shiftCount);
        }
    }
}

internal f32
f32_from_xf(u32 elemCount, u32 *x)
{
    u32 sign = xf_get_sign(elemCount, x);
    s32 exp = xf_unbiased_exponent(elemCount, x);
    u32 mantissa = x[XFLOAT_MANTISSA_IDX + 1];
    if ((exp > -127) && (exp < 128))
    {
        mantissa &= 0x7FFFFFFF; // NOTE(michiel): First bit is implied
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        mantissa = (mantissa >> 1) | (mantissa & 1);
        exp += 126;
    }
    else if (exp <= -127)
    {
        if (exp > (-127 - 23))
        {
            // TODO(michiel): Denormals
            mantissa = 0;
            exp = 0;
        }
        else
        {
            // NOTE(michiel): Zero
            mantissa = 0;
            exp = 0;
        }
    }
    else
    {
        // NOTE(michiel): Infinite
        mantissa = 0;
        exp = 0xFF;
    }

    u32 result = (((sign << 31) & F32_SIGN_MASK) |
                  ((exp  << 23) & F32_EXP_MASK)  |
                  (mantissa & F32_FRAC_MASK));
    return *(f32 *)&result;
}

internal f64
f64_from_xf(u32 elemCount, u32 *x)
{
    u64 sign = (u64)xf_get_sign(elemCount, x) << 32;
    s64 exp = xf_unbiased_exponent(elemCount, x);
    u64 mantissa = (((u64)x[XFLOAT_MANTISSA_IDX + 1] << 32) |
                    ((u64)x[XFLOAT_MANTISSA_IDX + 2]));
    if ((exp > -1023) && (exp < 1025))
    {
        mantissa &= 0x7FFFFFFFFFFFFFFF; // NOTE(michiel): First bit is implied
        exp += 1023;
    }
    else if (exp <= -1023)
    {
        if (exp > (-1023 - 52))
        {
            // TODO(michiel): Denormals
            mantissa = 0;
            exp = 0;
        }
        else
        {
            // NOTE(michiel): Zero
            mantissa = 0;
            exp = 0;
        }
    }
    else
    {
        // NOTE(michiel): Infinite
        mantissa = 0;
        exp = 0x7FF;
    }

    u64 result = (((sign << 63) & F64_SIGN_MASK) |
                  ((exp  << 52) & F64_EXP_MASK)  |
                  (mantissa & F64_FRAC_MASK));
    return *(f64 *)&result;
}

internal void
xf_print_raw(u32 elemCount, u32 *x, b32 newLine /* = true */)
{
    for (u32 elemIdx = 0; elemIdx < elemCount; ++elemIdx)
    {
        fprintf(stdout, "%s%08X", elemIdx ? "" : "0x", x[elemIdx]);
    }
    if (newLine) {
        fprintf(stdout, "\n");
    }
}

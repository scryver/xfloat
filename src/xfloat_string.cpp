internal void
xf_from_string(u32 elemCount, String string, u32 *x)
{
    u32 tempX[elemCount + 1];
    u32 accum[elemCount + 1];

    b32 parseError = false;

    xf_clear(elemCount + 1, tempX);

    s32 base = 10;
    if ((string.size > 1) &&
        (string.data[0] == '0') &&
        (to_lower_case(string.data[1]) == 'x'))
    {
        base = 16;
        string = advance(string, 2);
    }
    else if ((string.size > 1) &&
             (string.data[0] == '0') &&
             (to_lower_case(string.data[1]) == 'b'))
    {
        base = 2;
        string = advance(string, 2);
    }
    else if ((string.size > 1) &&
             (string.data[0] == '0') &&
             is_digit(string.data[1]))
    {
        base = 8;
        string = advance(string, 2);
    }

    u32 numberSign = 0;
    if (string.size && (string.data[0] == '-'))
    {
        numberSign = XFLOAT_SIGN_MASK;
        string = advance(string, 1);
    }
    else if (string.size && (string.data[0] == '+'))
    {
        string = advance(string, 1);
    }

    if (!is_hex_digit(string.data[0]))
    {
        parseError = true;
    }

    s32 precision = 0;
    s32 numberExponent = 0;
    s32 decimalPoint = 0;
    while (!parseError && string.size && is_hex_digit(string.data[0]))
    {
        u32 k = parse_half_hex_byte(string.data[0]);

        if (k < base)
        {
            if ((precision != 0) || decimalPoint || (string.data[0] != '0'))
            {
                if (precision < XFLOAT_MAX_DEC(elemCount))
                {
                    /* count digits after decimal point (if decimalPoint is set) */
                    numberExponent += decimalPoint;

                    if (base == 16)
                    {
                        xf_shift_up4(elemCount + 1, tempX);
                    }
                    else
                    {
                        xf_shift_up1(elemCount + 1, tempX); /* multiply current number by 10 */
                        xf_copy_extend(elemCount, tempX, accum);
                        xf_shift_up2(elemCount + 1, tempX);
                        xf_add_mantissa(elemCount + 1, accum, tempX);
                    }
                    xf_clear(elemCount + 1, accum);
                    accum[elemCount - 1] = k;
                    xf_add_mantissa(elemCount + 1, accum, tempX);
                }
                ++precision;
            }
        }
        else if ((base != 16) && to_lower_case(string.data[0]) == 'e')
        {
            break;
        }
        else
        {
            //fprintf(stderr, "Could not parse '%c' in base %u\n", string.data[0], base);
            parseError = true;
        }

        string = advance(string, 1);

        if (string.data[0] == '.')
        {
            if (decimalPoint) {
                parseError = true;
            } else {
                decimalPoint = 1;
                string = advance(string, 1);
            }
        }
    }

    s32 exp = 0;

    u32 doneFlags = parseError ? XFloatCompletion_Underflow : 0;

    s32 esign = 1;
    if (!doneFlags &&
        string.size &&
        ((to_lower_case(string.data[0]) == 'e') ||
         (to_lower_case(string.data[0]) == 'p')))
    {
        b32 isZero = true;
        string = advance(string, 1);

        /* 0.0eXXX is zero, regardless of XXX.  Check for the 0.0. */
        for (u32 index = 0; index < elemCount + 1; ++index)
        {
            if (tempX[index] != 0) {
                isZero = false;
                break;
            }
        }

        if (!isZero)
        {
            /* check for + or - */
            if (string.size && string.data[0] == '-')
            {
                esign = -1;
                string = advance(string, 1);
            }
            else if (string.size && string.data[0] == '+')
            {
                string = advance(string, 1);
            }

            while (!parseError && string.size && is_digit(string.data[0]))
            {
                /* Check for oversize decimal exponent.  */
                if (exp >= 3276 || exp < 0)
                {
                    if (esign < 0) {
                        doneFlags |= XFloatCompletion_Underflow;
                    } else {
                        doneFlags |= XFloatCompletion_Overflow;
                    }
                    break;
                }
                exp *= 10;
                exp += string.data[0] & 0xF;
                string = advance(string, 1);
            }

            if (esign < 0) {
                exp = -exp;
            }
        }
        else
        {
            doneFlags |= XFloatCompletion_Underflow;
        }
    }

    if (!doneFlags)
    {
        s32 shiftCount;
        if (base == 16)
        {
            /* Base 16 hexadecimal floating constant.  */
            xf_normalize_mantissa(elemCount, tempX, &shiftCount);
            if (shiftCount > XFLOAT_MAX_BITS(elemCount))
            {
                doneFlags |= XFloatCompletion_Underflow;
            }
            else
            {
                /* Adjust the exponent.  NEXP is the number of hex digits, EXP is a power of 2.  */
                s64 lexp = (XFLOAT_EXP_BIAS + XFLOAT_MAX_BITS(elemCount)) - shiftCount + xf_get_exponent(elemCount, tempX) + exp - 4 * numberExponent;

                if (lexp > XFLOAT_MAX_EXPONENT)  {
                    doneFlags |= XFloatCompletion_Overflow;
                }
                else if (lexp < 0) {
                    doneFlags |= XFloatCompletion_Underflow;
                }
                else
                {
                    tempX[XFLOAT_SIGN_EXP_IDX] = XFLOAT_SIGN_EXPONENT(numberSign, lexp);
                    doneFlags |= XFloatCompletion_Copy;
                }
            }
        }
        else
        {
            numberExponent = exp - numberExponent;

            if (xf_normalize_mantissa(elemCount, tempX, &shiftCount))
            {
                doneFlags |= XFloatCompletion_Underflow;
            }
            else
            {
                /* Escape from excessively large exponent.  */
                if (numberExponent >= 2 * XFLOAT_MAX_NTEN)
                {
                    doneFlags |= XFloatCompletion_Overflow;
                }
                else if (numberExponent <= -2 * XFLOAT_MAX_NTEN)
                {
                    doneFlags |= XFloatCompletion_Underflow;
                }
                else
                {
                    xf_set_exponent(elemCount, tempX, XFLOAT_EXP_BIAS + XFLOAT_MAX_BITS(elemCount) - shiftCount);
                    tempX[XFLOAT_SIGN_EXP_IDX] |= numberSign;
                    doneFlags |= XFloatCompletion_Copy;

                    /* multiply or divide by 10**NEXP */
                    if (numberExponent != 0) {
                        esign = 0;
                        if (numberExponent < 0)
                        {
                            esign = -1;
                            numberExponent = -numberExponent;
                        }

                        u32 *p = xf_get_power_of_ten(elemCount, 0); // &gXF_Tens[0][0];
                        exp = 1;
                        xf_copy(elemCount, gXF_One, accum);
                        accum[elemCount] = 0;

                        do
                        {
                            if (exp & numberExponent) {
                                xf_mul(elemCount, p, accum, accum);
                            }
                            exp <<= 1;
                            p += elemCount;
                        }
                        while (exp <= XFLOAT_MAX_NTEN);

                        if (esign < 0) {
                            xf_div(elemCount, tempX, accum, x);
                        } else {
                            xf_mul(elemCount, accum, tempX, x);
                        }
                        doneFlags = 0;
                    }
                }
            }
        }
    }

    if (doneFlags & XFloatCompletion_Underflow)
    {
        xf_clear(elemCount, x);
    }
    else if (doneFlags & XFloatCompletion_Overflow)
    {
        x[XFLOAT_SIGN_EXP_IDX] |= numberSign;
        xf_infinite(elemCount, x);
    }
    else if (doneFlags & XFloatCompletion_Copy)
    {
        xf_copy(elemCount, tempX, x);
    }
}

internal String
string_from_xf(u32 elemCount, u32 *x, u32 digits, u32 maxDataCount, u8 *data)
{
    String result = {};
    if (xf_is_infinite(elemCount, x))
    {
        result = string_fmt(maxDataCount, data, "%sINF", xf_get_sign(elemCount, x) ? "-" : "");
    }
    else if (!xf_get_exponent(elemCount, x))
    {
        result = string_fmt(maxDataCount, data, "0.0E0");
    }
    else
    {
        u32 accum[elemCount + 1];
        u32 xc[elemCount];
        u32 xt[elemCount];

        xf_copy(elemCount, x, xc);
        u32 sign = xf_get_sign(elemCount, xc);
        xf_make_positive(elemCount, xc);
        s32 exponent = 0;
        u32 *ten = xf_get_power_of_ten(elemCount, 0); // &gXF_Tens[0][0];

        s32 i = xf_compare(elemCount, gXF_One, xc);
        if (i != 0)
        {
            if (xf_get_exponent(elemCount, xc) != 0)
            {
                if (i < 0)
                {
                    s32 k = XFLOAT_MAX_NTEN;
                    u32 *p = xf_get_power_of_ten(elemCount, XFLOAT_NUMBER_TENS); // &gXF_Tens[XFLOAT_NUMBER_TENS][0];
                    xf_copy(elemCount, gXF_One, accum);
                    xf_copy(elemCount, xc, xt);
                    while (xf_compare(elemCount, ten, xc) <= 0)
                    {
                        if (xf_compare(elemCount, p, xt) <= 0)
                        {
                            xf_div(elemCount, xt, p, xt);
                            xf_mul(elemCount, p, accum, accum);
                            exponent += k;
                        }
                        k >>= 1;

                        if (k == 0) {
                            break;
                        }
                        p -= elemCount;
                    }
                    xf_div(elemCount, xc, accum, xc);
                }
                else
                {
                    s32 k = XFLOAT_MIN_NTEN;
                    u32 *p = xf_get_power_of_tenths(elemCount, XFLOAT_NUMBER_TENS); // &gXF_Tenths[XFLOAT_NUMBER_TENS][0];
                    u32 *r = xf_get_power_of_ten(elemCount, XFLOAT_NUMBER_TENS); // &gXF_Tens[XFLOAT_NUMBER_TENS][0];
                    u32 *tenth = xf_get_power_of_tenths(elemCount, 0); // &gXF_Tenths[0][0];
                    while (xf_compare(elemCount, tenth, xc) > 0)
                    {
                        if (xf_compare(elemCount, p, xc) >= 0)
                        {
                            xf_mul(elemCount, r, xc, xc);
                            exponent += k;
                        }
                        k /= 2;

                        if (k == 0) {
                            break;
                        }
                        p -= elemCount;
                        r -= elemCount;
                    }
                    xf_multiply_int(elemCount, ten, xc, xc);
                    exponent -= 1;
                }
            }
            else
            {
                xf_clear(elemCount, xc);
            }
        }

        s64 digit = xf_integer_fraction(elemCount, xc, xc);
        if (digit >= 10)
        {
            xf_div(elemCount, xc, ten, xc);
            ++exponent;
            digit = 1;
        }

        char *s = (char *)data;
        if (sign != 0) {
            *s++ = '-';
        } else {
            *s++ = ' ';
        }

        *s++ = (char)digit | 0x30;
        *s++ = '.';

        if (digits < 0) {
            digits = 0;
        }
        if (digits > XFLOAT_MAX_DEC(elemCount)) {
            digits = XFLOAT_MAX_DEC(elemCount);
        }
        if (digits > (maxDataCount - 3)) {
            digits = maxDataCount - 3; // NOTE(michiel): 1 for sign, 1 for first digit and 1 for the dot
        }

        for (u32 k = 0; k < digits; ++k)
        {
            xf_multiply_int(elemCount, ten, xc, xc);
            digit = xf_integer_fraction(elemCount, xc, xc);
            *s++ = (char)digit | 0x30;
        }

        *s = '\0';
        char *ss = s;

        xf_multiply_int(elemCount, ten, xc, xc);
        digit = xf_integer_fraction(elemCount, xc, xc);
        if (digit > 4)
        {
            b32 done = false;
            if (digit == 5)
            {
                if (xf_compare(elemCount, xc, gXF_Zero) == 0)
                {
                    if ((*(s - 1) & 0x1) == 0)
                    {
                        done = true;
                    }
                }
            }

            while (!done)
            {
                --s;
                s32 k = *s & 0x7F;
                if (k == '.')
                {
                    --s;
                    k = *s & 0x7F;
                    k += 1;
                    *s = k;
                    if (k > '9')
                    {
                        *s = '1';
                        ++exponent;
                    }
                    done = true;
                }
                else
                {
                    k += 1;
                    *s = k;
                    if (k > '9')
                    {
                        *s = '0';
                    }
                    else
                    {
                        done = true;
                    }
                }
            }
        }

        sprintf(ss, "E%d", exponent);

        result = string((char *)data);
    }

    return result;
}

internal void
xf_print(u32 elemCount, u32 *x, u32 digits = U32_MAX)
{
    u8 stringBuf[512];
    String xStr = string_from_xf(elemCount, x, digits, array_count(stringBuf), stringBuf);
    fprintf(stdout, "%.*s", STR_FMT(xStr));
}

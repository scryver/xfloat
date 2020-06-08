internal void
xf_tst_square_root(u32 elemCount, u32 *src, u32 *dst, u32 iterations = 8)
{
    if (xf_get_exponent(elemCount, src))
    {
        if (!xf_get_sign(elemCount, src))
        {
            u32 source[elemCount];
            u32 fract[elemCount];

            xf_copy(elemCount, src, source);
            xf_copy(elemCount, src, fract);

            s32 exponent = (s32)xf_unbiased_exponent(elemCount, src);
            xf_set_exponent(elemCount, fract, XFLOAT_EXP_BIAS);

            xf_multiply(elemCount, gXF_SquareRootCoef2, fract, dst);
            xf_add(elemCount, gXF_SquareRootCoef1, dst, dst);
            xf_multiply(elemCount, fract, dst, dst);
            xf_add(elemCount, gXF_SquareRootCoef0, dst, dst);

            xf_set_exponent(elemCount, dst, (exponent / 2) + XFLOAT_EXP_BIAS);

            if (exponent & 1)
            {
                // NOTE(michiel): Exponent is odd
                xf_multiply(elemCount, dst, gXF_Sqrt2, dst);
            }

            for (u32 iterIdx = 0; iterIdx < iterations; ++iterIdx)
            {
                xf_divide(elemCount, source, dst, fract);
                xf_add(elemCount, fract, dst, dst);
                xf_naive_div2(elemCount, dst);
            }
        }
        else
        {
            // NOTE(michiel): X < 0
            // NOTE(michiel): Domain error (imaginary number)
            xf_clear(elemCount, dst);
        }
    }
    else
    {
        // NOTE(michiel): X == 0
        xf_clear(elemCount, dst);
    }
}

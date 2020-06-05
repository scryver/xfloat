internal void
xf_random(RandomSeriesPCG *series, u32 elemCount, u32 *dst)
{
    u32 nextRand = random_next_u32(series);
    u32 sign = nextRand & XFLOAT_SIGN_MASK;
    u32 exponent = nextRand & XFLOAT_MAX_EXP_MASK;
    dst[XFLOAT_SIGN_EXP_IDX] = sign | exponent;
    dst[XFLOAT_MANTISSA_IDX] = 0;
    for (u32 mantissaIdx = 0; mantissaIdx < (elemCount - 2); ++mantissaIdx)
    {
        dst[XFLOAT_MANTISSA_IDX + mantissaIdx + 1] = random_next_u32(series);
    }

    dst[XFLOAT_MANTISSA_IDX + 1] |= XFLOAT_HIGH_BIT;
}

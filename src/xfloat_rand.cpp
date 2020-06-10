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

internal void
xf_random_in_exp_range(RandomSeriesPCG *series, u32 elemCount, s32 minExp, s32 maxExp, u32 *dst)
{
    xf_random(series, elemCount, dst);
    if (xf_unbiased_exponent(elemCount, dst) > maxExp)
    {
        xf_set_exponent(elemCount, dst, XFLOAT_EXP_BIAS + maxExp);
    }
    if (xf_unbiased_exponent(elemCount, dst) < minExp)
    {
        xf_set_exponent(elemCount, dst, XFLOAT_EXP_BIAS + minExp);
    }
}

internal void
xf_random_bilateral(RandomSeriesPCG *series, u32 elemCount, u32 *dst)
{
    xf_random(series, elemCount, dst);
    u32 exponent = xf_get_exponent(elemCount, dst);
    if (exponent > XFLOAT_EXP_BIAS)
    {
        xf_set_exponent(elemCount, dst, exponent - XFLOAT_EXP_BIAS);
    }
}

internal void
xf_random_unilateral(RandomSeriesPCG *series, u32 elemCount, u32 *dst)
{
    xf_random_bilateral(series, elemCount, dst);
    xf_make_positive(elemCount, dst);
}

internal void
xf_random_map(RandomSeriesPCG *series, u32 elemCount, u32 *offset, u32 *range, u32 *dst)
{
    // NOTE(michiel): dst = offset + range * random(0,1)
    xf_random_unilateral(series, elemCount, dst);
    xf_mul(elemCount, dst, range, dst);
    xf_add(elemCount, dst, offset, dst);
}

internal void
xf_random_log(RandomSeriesPCG *series, u32 elemCount, u32 *x, u32 *dst)
{
    // NOTE(michiel): range from 1 to exp(x)
    // So you can use A * xf_random_log(log(B/A)) which is logarithmically distributed
    // over (A, B)

    xf_random_unilateral(series, elemCount, dst);
    xf_mul(elemCount, x, dst, dst);
    xf_exp(elemCount, dst, dst);
}

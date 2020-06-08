#include <time.h>

#include "../libberdip/src/platform.h"
#include "../libberdip/src/random.h"

#include "xfloat.h"
#include "xfloat_math.h"
#include "xfloat.cpp"
#include "xfloat_constants_16.cpp"
#include "xfloat_string.cpp"
#include "xfloat_math.cpp"
#include "xfloat_rand.cpp"

#define ELEMENT_COUNT   16

struct Stats
{
    u32 minErr[ELEMENT_COUNT];
    u32 avgErr[ELEMENT_COUNT];
    u32 rmsErr[ELEMENT_COUNT];
    u32 maxErr[ELEMENT_COUNT];
    u32 inpMin[ELEMENT_COUNT];
    u32 inpMax[ELEMENT_COUNT];
};

internal void
begin_error_stats(u32 elemCount, Stats *stats)
{
    i_expect(elemCount == array_count(stats->minErr)); // TODO(michiel): Maybe make stats dynamic
    xf_max_value(elemCount, stats->minErr);
    xf_clear(elemCount, stats->avgErr);
    xf_clear(elemCount, stats->maxErr);
    xf_clear(elemCount, stats->inpMin);
    xf_clear(elemCount, stats->inpMax);
}

internal void
end_error_stats(u32 elemCount, Stats *stats, u32 testCount)
{
    u32 divider[elemCount];
    // TODO(michiel): Make a xf_from_u32
    xf_from_s32(elemCount, testCount, divider);

    xf_copy(elemCount, stats->avgErr, stats->rmsErr);
    xf_multiply(elemCount, stats->rmsErr, stats->rmsErr, stats->rmsErr);
    xf_divide(elemCount, stats->rmsErr, divider, stats->rmsErr);
    // NOTE(michiel): RMS Error of square_root is only useful if the function is good
    xf_square_root(elemCount, stats->rmsErr, stats->rmsErr);

    xf_divide(elemCount, stats->avgErr, divider, stats->avgErr);
}

internal void
accum_error_stats(u32 elemCount, u32 *error, u32 *input, Stats *stats)
{
    if (xf_compare(elemCount, stats->minErr, error) > 0)
    {
        xf_copy(elemCount, error, stats->minErr);
        xf_copy(elemCount, input, stats->inpMin);
    }
    if (xf_compare(elemCount, stats->maxErr, error) < 0)
    {
        xf_copy(elemCount, error, stats->maxErr);
        xf_copy(elemCount, input, stats->inpMax);
    }
    xf_add(elemCount, error, stats->avgErr, stats->avgErr);
}

internal void
print_error_stats(u32 elemCount, Stats *stats)
{
    u8 errorBuf[512];
    u8 inputBuf[512];

    String errorStr = string_from_xf(ELEMENT_COUNT, stats->minErr, U32_MAX, array_count(errorBuf), errorBuf);
    String inputStr = string_from_xf(ELEMENT_COUNT, stats->inpMin, U32_MAX, array_count(inputBuf), inputBuf);
    //fprintf(stdout, "  Min error: %.*s\n", STR_FMT(errorStr));
    //fprintf(stdout, "     @ input %.*s\n", STR_FMT(inputStr));

    errorStr = string_from_xf(ELEMENT_COUNT, stats->maxErr, U32_MAX, array_count(errorBuf), errorBuf);
    inputStr = string_from_xf(ELEMENT_COUNT, stats->inpMax, U32_MAX, array_count(inputBuf), inputBuf);
    fprintf(stdout, "  Max error: %.*s\n", STR_FMT(errorStr));
    //fprintf(stdout, "     @ input %.*s\n", STR_FMT(inputStr));

    //errorStr = string_from_xf(ELEMENT_COUNT, stats->avgErr, U32_MAX, array_count(errorBuf), errorBuf);
    //fprintf(stdout, "  Avg error: %.*s\n", STR_FMT(errorStr));

    errorStr = string_from_xf(ELEMENT_COUNT, stats->rmsErr, U32_MAX, array_count(errorBuf), errorBuf);
    fprintf(stdout, "  RMS error: %.*s\n", STR_FMT(errorStr));

    if (xf_compare(elemCount, stats->maxErr, gXF_Zero) > 0)
    {
        u32 totalBits[elemCount];
        u32 bits[elemCount];
        xf_log2(elemCount, stats->maxErr, bits);
        xf_from_s32(elemCount, XFLOAT_MAX_BITS(elemCount), totalBits);
        if (xf_compare(elemCount, bits, totalBits) > 0)
        {
            xf_clear(elemCount, bits);
        }
        else
        {
            xf_add(elemCount, totalBits, bits, bits);
        }

        errorStr = string_from_xf(ELEMENT_COUNT, bits, U32_MAX, array_count(errorBuf), errorBuf);
        fprintf(stdout, "  ERROR BITS: %.*s\n", STR_FMT(errorStr));
    }
    else
    {
        fprintf(stdout, "  ERROR BITS: 0\n");
    }
}

//
// NOTE(michiel): Test square_root
//
internal void
test_square_root_base_error(u32 testCount, Stats *errorStats)
{
    u32 input[ELEMENT_COUNT];
    u32 error[ELEMENT_COUNT];

    u64 timeRand = time(NULL);
    fprintf(stdout, "test_square_root_base_error(%u): time rand %lu\n", testCount, timeRand);

    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 19824701273409LL);
    RandomSeriesPCG *series = &series_;

    begin_error_stats(ELEMENT_COUNT, errorStats);

    // NOTE(michiel): (r, s) => r = sqrt(1/2), s = 1
    u32 inputOffset[ELEMENT_COUNT];
    u32 inputRange[ELEMENT_COUNT];
    xf_divide(ELEMENT_COUNT, gXF_One, gXF_Sqrt2, inputOffset);
    xf_subtract(ELEMENT_COUNT, gXF_One, inputOffset, inputRange);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        xf_multiply(ELEMENT_COUNT, input, input, error);

        xf_square_root(ELEMENT_COUNT, error, error, 6);

        xf_subtract(ELEMENT_COUNT, error, input, error);
        xf_absolute(ELEMENT_COUNT, error);
        xf_divide(ELEMENT_COUNT, error, input, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

internal void
test_square_root_1_to_2(u32 testCount, Stats *errorStats)
{
    u32 input[ELEMENT_COUNT];
    u32 error[ELEMENT_COUNT];

    u64 timeRand = time(NULL);
    fprintf(stdout, "test_square_root_1_to_2(%u): time rand %lu\n", testCount, timeRand);

    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 19824701273409LL);
    RandomSeriesPCG *series = &series_;

    begin_error_stats(ELEMENT_COUNT, errorStats);

    // NOTE(michiel): (r, s) => r = 1, s = sqrt(2)
    u32 inputOffset[ELEMENT_COUNT];
    u32 inputRange[ELEMENT_COUNT];
    xf_copy(ELEMENT_COUNT, gXF_One, inputOffset);
    xf_subtract(ELEMENT_COUNT, gXF_Sqrt2, gXF_One, inputRange);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        xf_multiply(ELEMENT_COUNT, input, input, error);

        xf_square_root(ELEMENT_COUNT, error, error, 6);

        xf_subtract(ELEMENT_COUNT, error, input, error);
        xf_absolute(ELEMENT_COUNT, error);
        xf_divide(ELEMENT_COUNT, error, input, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

int main(int argc, char **argv)
{
    Stats relStats;

    u32 testBit[ELEMENT_COUNT] = {};
    testBit[ELEMENT_COUNT - 1] = 0x1;
    s32 shiftCount;
    xf_normalize_mantissa(ELEMENT_COUNT, testBit, &shiftCount);
    u32 exponent = XFLOAT_EXP_BIAS;
    xf_set_exponent(ELEMENT_COUNT, testBit, exponent - shiftCount);

    u8 testBuf[512];
    String testStr = string_from_xf(ELEMENT_COUNT, testBit, U32_MAX, array_count(testBuf), testBuf);
    fprintf(stdout, "One bit: %.*s\n\n", STR_FMT(testStr));

    test_square_root_base_error(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    test_square_root_1_to_2(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    return 0;
}
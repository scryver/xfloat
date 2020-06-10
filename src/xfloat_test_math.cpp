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

#include "elementary_test.cpp"

struct Stats
{
    u32 minErr[ELEMENT_COUNT];
    u32 avgErr[ELEMENT_COUNT];
    u32 rmsErr[ELEMENT_COUNT];
    u32 maxErr[ELEMENT_COUNT];
    u32 inpMin[ELEMENT_COUNT];
    u32 inpMax[ELEMENT_COUNT];

    struct timespec start;
    struct timespec end;
};

internal struct timespec
get_wall_clock(void)
{
    struct timespec clock;
    clock_gettime(CLOCK_MONOTONIC, &clock);
    return clock;
}

internal f32
get_seconds_elapsed(struct timespec start, struct timespec end)
{
    return ((f32)(end.tv_sec - start.tv_sec)
            + ((f32)(end.tv_nsec - start.tv_nsec) * 1e-9f));
}

internal void
begin_error_stats(u32 elemCount, Stats *stats)
{
    i_expect(elemCount == array_count(stats->minErr)); // TODO(michiel): Maybe make stats dynamic
    xf_max_value(elemCount, stats->minErr);
    xf_clear(elemCount, stats->avgErr);
    xf_clear(elemCount, stats->rmsErr);
    xf_clear(elemCount, stats->maxErr);
    xf_clear(elemCount, stats->inpMin);
    xf_clear(elemCount, stats->inpMax);

    stats->start = get_wall_clock();
}

internal void
end_error_stats(u32 elemCount, Stats *stats, u32 testCount)
{
    stats->end = get_wall_clock();

    u32 divider[elemCount];
    // TODO(michiel): Make a xf_from_u32
    xf_from_s32(elemCount, testCount, divider);

    // NOTE(michiel): RMS Error of square_root is only useful if the square_root function is good
    xf_mul(elemCount, stats->avgErr, stats->avgErr, stats->rmsErr);
    xf_div(elemCount, stats->rmsErr, divider, stats->rmsErr);
    xf_square_root(elemCount, stats->rmsErr, stats->rmsErr);

    xf_div(elemCount, stats->avgErr, divider, stats->avgErr);
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
    //u32 sqrErr[elemCount];
    //xf_multiply(elemCount, error, error, sqrErr);
    //xf_add(elemCount, error, stats->rmsErr, stats->rmsErr);
}

internal void
print_error_stats(u32 elemCount, Stats *stats)
{
    u8 errorBuf[512];
    u8 inputBuf[512];

    u32 maxFraction = 10;

    fprintf(stdout, "  Completed in: %f seconds\n", get_seconds_elapsed(stats->start, stats->end));

    String errorStr = string_from_xf(ELEMENT_COUNT, stats->minErr, maxFraction, array_count(errorBuf), errorBuf);
    String inputStr = string_from_xf(ELEMENT_COUNT, stats->inpMin, maxFraction, array_count(inputBuf), inputBuf);
    //fprintf(stdout, "  Min error: %.*s\n", STR_FMT(errorStr));
    //fprintf(stdout, "     @ input %.*s\n", STR_FMT(inputStr));

    errorStr = string_from_xf(ELEMENT_COUNT, stats->maxErr, maxFraction, array_count(errorBuf), errorBuf);
    inputStr = string_from_xf(ELEMENT_COUNT, stats->inpMax, maxFraction, array_count(inputBuf), inputBuf);
    fprintf(stdout, "  Max error: %.*s\n", STR_FMT(errorStr));
    //fprintf(stdout, "     @ input %.*s\n", STR_FMT(inputStr));

    //errorStr = string_from_xf(ELEMENT_COUNT, stats->avgErr, maxFraction, array_count(errorBuf), errorBuf);
    //fprintf(stdout, "  Avg error: %.*s\n", STR_FMT(errorStr));

    errorStr = string_from_xf(ELEMENT_COUNT, stats->rmsErr, maxFraction, array_count(errorBuf), errorBuf);
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

        errorStr = string_from_xf(ELEMENT_COUNT, bits, 2, array_count(errorBuf), errorBuf);
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
    xf_div(ELEMENT_COUNT, gXF_One, gXF_Sqrt2, inputOffset);
    xf_sub(ELEMENT_COUNT, gXF_One, inputOffset, inputRange);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        xf_mul(ELEMENT_COUNT, input, input, error);

        xf_square_root(ELEMENT_COUNT, error, error, 6);

        xf_sub(ELEMENT_COUNT, error, input, error);
        xf_div(ELEMENT_COUNT, error, input, error);
        xf_absolute(ELEMENT_COUNT, error);

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
    xf_sub(ELEMENT_COUNT, gXF_Sqrt2, gXF_One, inputRange);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        xf_mul(ELEMENT_COUNT, input, input, error);

        xf_square_root(ELEMENT_COUNT, error, error, 6);

        xf_sub(ELEMENT_COUNT, error, input, error);
        xf_div(ELEMENT_COUNT, error, input, error);
        xf_absolute(ELEMENT_COUNT, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

//
// NOTE(michiel): Test log/log10
//

internal void
test_log_small_approx(u32 elemCount, u32 *src, u32 *dst)
{
    u32 x[elemCount];

    xf_sub(elemCount, gXF_One, src, x);

    u32 accumX[elemCount];
    u32 accumY[elemCount];
    u32 accumD[elemCount];
    xf_clear(elemCount, dst);
    xf_copy(elemCount, gXF_One, accumX);
    xf_copy(elemCount, gXF_One, accumY);
    do
    {
        // NOTE(michiel): (1-x)^N / N
        xf_mul(elemCount, accumX, x, accumX);
        xf_div(elemCount, accumX, accumY, accumD);
        xf_add(elemCount, accumD, dst, dst);
        xf_add(elemCount, gXF_One, accumY, accumY);
    } while ((s32)(xf_get_exponent(elemCount, dst) - xf_get_exponent(elemCount, accumD)) <= XFLOAT_MAX_BITS(elemCount));

    xf_negate(elemCount, dst);
}

internal void
test_log_close_to_1(u32 testCount, Stats *errorStats)
{
    u32 input[ELEMENT_COUNT];
    u32 error[ELEMENT_COUNT];
    u32 answer[ELEMENT_COUNT];

    u64 timeRand = time(NULL);
    fprintf(stdout, "test_log_close_to_1(%u): time rand %lu\n", testCount, timeRand);

    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 19824701273409LL);
    RandomSeriesPCG *series = &series_;

    begin_error_stats(ELEMENT_COUNT, errorStats);

    // NOTE(michiel): Close to 1
    u32 inputOffset[ELEMENT_COUNT];
    u32 inputRange[ELEMENT_COUNT];

    xf_from_string(ELEMENT_COUNT, static_string("0.9999"), inputOffset);
    xf_from_string(ELEMENT_COUNT, static_string("0.0002"), inputRange);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);

        xf_log(ELEMENT_COUNT, input, error);

        test_log_small_approx(ELEMENT_COUNT, input, answer);

        xf_sub(ELEMENT_COUNT, error, answer, error);
        xf_div(ELEMENT_COUNT, error, answer, error);
        xf_absolute(ELEMENT_COUNT, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

internal void
test_log_17_over_16(u32 testCount, Stats *errorStats)
{
    u32 input[ELEMENT_COUNT];
    u32 error[ELEMENT_COUNT];
    u32 answer[ELEMENT_COUNT];

    u64 timeRand = time(NULL);
    fprintf(stdout, "test_log_17_over_16(%u): time rand %lu\n", testCount, timeRand);

    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 19824701273409LL);
    RandomSeriesPCG *series = &series_;

    begin_error_stats(ELEMENT_COUNT, errorStats);

    // NOTE(michiel): ln(x) = ln(x*17/16) - ln(17/16)
    // 1/sqrt(2) <= x <= 15/16
    u32 inputOffset[ELEMENT_COUNT];
    u32 inputRange[ELEMENT_COUNT];

    xf_from_s32(ELEMENT_COUNT, 15, inputRange);
    xf_from_s32(ELEMENT_COUNT, 16, inputOffset);
    xf_div(ELEMENT_COUNT, inputRange, inputOffset, inputRange);
    xf_div(ELEMENT_COUNT, gXF_One, gXF_Sqrt2, inputOffset);
    xf_sub(ELEMENT_COUNT, inputRange, inputOffset, inputRange);

    u32 seventeen[ELEMENT_COUNT];
    u32 sixteen[ELEMENT_COUNT];
    u32 log17Over16[ELEMENT_COUNT];

    xf_from_s32(ELEMENT_COUNT, 17, seventeen);
    xf_from_s32(ELEMENT_COUNT, 16, sixteen);
    xf_div(ELEMENT_COUNT, seventeen, sixteen, log17Over16);
    xf_log(ELEMENT_COUNT, log17Over16, log17Over16);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        input[ELEMENT_COUNT - 1] &= 0xFFFFFFF0; // NOTE(michiel): Make sure lowest 4 bits are zero

        xf_log(ELEMENT_COUNT, input, error); // ln(x)

        xf_mul(ELEMENT_COUNT, input, seventeen, answer);    // 17*x
        xf_div(ELEMENT_COUNT, answer, sixteen, answer);       // 17x/16
        xf_log(ELEMENT_COUNT, answer, answer);                   // log(17x/16)
        xf_sub(ELEMENT_COUNT, answer, log17Over16, answer); // log(17x/16)-log(17/16)

        xf_sub(ELEMENT_COUNT, error, answer, answer);
        xf_div(ELEMENT_COUNT, answer, error, error);
        xf_absolute(ELEMENT_COUNT, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

internal void
test_log10_11_over_10(u32 testCount, Stats *errorStats)
{
    u32 input[ELEMENT_COUNT];
    u32 error[ELEMENT_COUNT];
    u32 answer[ELEMENT_COUNT];

    u64 timeRand = time(NULL);
    fprintf(stdout, "test_log10_11_over_10(%u): time rand %lu\n", testCount, timeRand);

    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 19824701273409LL);
    RandomSeriesPCG *series = &series_;

    begin_error_stats(ELEMENT_COUNT, errorStats);

    // NOTE(michiel): log10(x) = log10(x*11/10) - log10(1.1)
    // 1/sqrt(10) <= x <= 0.9
    u32 inputOffset[ELEMENT_COUNT];
    u32 inputRange[ELEMENT_COUNT];

    xf_copy(ELEMENT_COUNT, &gXF_Tens[0][0], inputOffset);
    xf_square_root(ELEMENT_COUNT, inputOffset, inputOffset);
    xf_div(ELEMENT_COUNT, gXF_One, inputOffset, inputOffset);

    xf_sub(ELEMENT_COUNT, gXF_One, &gXF_Tenths[0][0], inputRange);

    u32 eleven[ELEMENT_COUNT];
    u32 ten[ELEMENT_COUNT];
    u32 log1p1[ELEMENT_COUNT];

    xf_from_s32(ELEMENT_COUNT, 11, eleven);
    xf_copy(ELEMENT_COUNT, &gXF_Tens[0][0], ten);
    xf_div(ELEMENT_COUNT, eleven, ten, log1p1);
    xf_log10(ELEMENT_COUNT, log1p1, log1p1);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        input[ELEMENT_COUNT - 1] &= 0xFFFFFFF0; // NOTE(michiel): Make sure lowest 4 bits are zero

        xf_log10(ELEMENT_COUNT, input, error); // log10(x)

        xf_mul(ELEMENT_COUNT, input, eleven, answer);   // 11*x
        xf_div(ELEMENT_COUNT, answer, ten, answer);       // 11x/10
        xf_log10(ELEMENT_COUNT, answer, answer);             // log10(11x/10)
        xf_sub(ELEMENT_COUNT, answer, log1p1, answer);  // log10(11x/10)-log(11/10)

        xf_sub(ELEMENT_COUNT, error, answer, answer);
        xf_div(ELEMENT_COUNT, answer, error, error);
        xf_absolute(ELEMENT_COUNT, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

internal void
test_log_extended_range(u32 testCount, Stats *errorStats)
{
    u32 input[ELEMENT_COUNT];
    u32 error[ELEMENT_COUNT];
    u32 answer[ELEMENT_COUNT];

    u64 timeRand = time(NULL);
    fprintf(stdout, "test_log_extended_range(%u): time rand %lu\n", testCount, timeRand);

    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 19824701273409LL);
    RandomSeriesPCG *series = &series_;

    begin_error_stats(ELEMENT_COUNT, errorStats);

    // NOTE(michiel): ln(x^2) = 2ln(x)
    // 16 <= x <= 240
    u32 inputOffset[ELEMENT_COUNT];
    u32 inputRange[ELEMENT_COUNT];

    xf_from_s32(ELEMENT_COUNT, 16, inputOffset);
    xf_from_s32(ELEMENT_COUNT, 240, inputRange);
    xf_sub(ELEMENT_COUNT, inputRange, inputOffset, inputRange);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        input[ELEMENT_COUNT - 1] &= 0xFFFFFFF0; // NOTE(michiel): Make sure lowest 4 bits are zero

        xf_mul(ELEMENT_COUNT, input, input, error);
        xf_log(ELEMENT_COUNT, error, error); // log(x^2)

        xf_log(ELEMENT_COUNT, input, answer);
        xf_mul(ELEMENT_COUNT, gXF_Two, answer, answer);

        xf_sub(ELEMENT_COUNT, error, answer, answer);
        xf_div(ELEMENT_COUNT, answer, error, error);
        xf_absolute(ELEMENT_COUNT, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

//
// NOTE(michiel): Test exp
//

internal void
test_exp_base_error(u32 testCount, Stats *errorStats)
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
    xf_div(ELEMENT_COUNT, gXF_One, gXF_Sqrt2, inputOffset);
    xf_sub(ELEMENT_COUNT, gXF_One, inputOffset, inputRange);

    u32 oneOverSixteen[ELEMENT_COUNT];
    xf_copy(ELEMENT_COUNT, gXF_One, oneOverSixteen);
    xf_set_exponent(ELEMENT_COUNT, oneOverSixteen, XFLOAT_EXP_BIAS - 3);

    for (u32 test = 0; test < testCount; ++test)
    {
        xf_random_map(series, ELEMENT_COUNT, inputOffset, inputRange, input);
        xf_mul(ELEMENT_COUNT, input, input, error);

        xf_square_root(ELEMENT_COUNT, error, error, 6);

        xf_sub(ELEMENT_COUNT, error, input, error);
        xf_div(ELEMENT_COUNT, error, input, error);
        xf_absolute(ELEMENT_COUNT, error);

        accum_error_stats(ELEMENT_COUNT, error, input, errorStats);
    }

    end_error_stats(ELEMENT_COUNT, errorStats, testCount);
}

int main(int argc, char **argv)
{
    u32 testBit[ELEMENT_COUNT] = {};
    testBit[ELEMENT_COUNT - 1] = 0x1;
    s32 shiftCount;
    xf_normalize_mantissa(ELEMENT_COUNT, testBit, &shiftCount);
    u32 exponent = XFLOAT_EXP_BIAS;
    xf_set_exponent(ELEMENT_COUNT, testBit, exponent - shiftCount);

    u8 testBuf[512];
    String testStr = string_from_xf(ELEMENT_COUNT, testBit, U32_MAX, array_count(testBuf), testBuf);
    fprintf(stdout, "One bit: %.*s\n\n", STR_FMT(testStr));

#if 0
    Stats relStats;

    // Sqrt

    test_square_root_base_error(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    test_square_root_1_to_2(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    // Log

    test_log_close_to_1(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    test_log_17_over_16(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    test_log10_11_over_10(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    test_log_extended_range(10000, &relStats);
    print_error_stats(ELEMENT_COUNT, &relStats);

    // Exp
#endif


    //
    //
    //

    Arena arena = {};
    Environment env = {};
    init_environment(&env, ELEMENT_COUNT, &arena);

    test_square_root(&env, ELEMENT_COUNT);
    test_log(&env, ELEMENT_COUNT);

    return 0;
}

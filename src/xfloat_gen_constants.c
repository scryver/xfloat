//#include "../libberdip/src/platform.h"
#include "../libberdip/src/common.h"
#include <math.h>
#define pow(...) powf(__VA_ARGS__)
#include "../libberdip/src/maps.h"
#include "../libberdip/src/strings.h"

#ifndef EXTENDED_PRECISION_FULL
#define EXTENDED_PRECISION_FULL  0
#endif

global s32 XFLOAT_NUMBER_TENS;
global s32 XFLOAT_MIN_NTEN;
global s32 XFLOAT_MAX_NTEN;

global u32 *gXF_Tens;
global u32 *gXF_Tenths;

internal u32 *
xf_get_power_of_ten(u32 elemCount, u32 index)
{
    i_expect(index <= XFLOAT_NUMBER_TENS);
    return gXF_Tens + elemCount * index;
}

internal u32 *
xf_get_power_of_tenths(u32 elemCount, u32 index)
{
    i_expect(index <= XFLOAT_NUMBER_TENS);
    return gXF_Tenths + elemCount * index;
}

// TODO(michiel): Make the Upper/Lower version have even more extra bits??

global u32 *gXF_Zero;
global u32 *gXF_One;
global u32 *gXF_Half;
global u32 *gXF_Two;
global u32 *gXF_Three;
global u32 *gXF_Four;
global u32 *gXF_Five;
global u32 *gXF_Six;
global u32 *gXF_Seven;
global u32 *gXF_Eight;
global u32 *gXF_Nine;
global u32 *gXF_NegativeTwo;
global u32 *gXF_Sqrt2;
global u32 *gXF_Pi;
global u32 *gXF_PiOver2;

global u32 *gXF_Log2;
global u32 *gXF_Log10;
global u32 *gXF_Log2e;
global u32 *gXF_Log10e;

global u32 *gXF_TanPiOver8;
global u32 *gXF_Tan3PiOver8;

global u32 *gXF_PiOver2Upper;
global u32 *gXF_PiOver2Lower;

global u32 *gXF_Log2Upper;
global u32 *gXF_Log2Lower;
global u32 *gXF_Log2eUpper;
global u32 *gXF_Log2eLower;
global u32 *gXF_Log10eUpper;
global u32 *gXF_Log10eLower;

global u32 *gXF_SquareRootCoef0;
global u32 *gXF_SquareRootCoef1;
global u32 *gXF_SquareRootCoef2;

#include "xfloat.h"
#include "xfloat_math.h"
#include "xfloat.c"
#include "xfloat_math.c"
#include "xfloat_string.c"

#include "xfloat_gen_strings.c"

// NOTE(michiel): This can generate the values needed to parse strings as xfloats and outputs them to a file.
// It will also parse in common values from xfloat_gen_strings

#define generate_value(f, e, v)                            generate_value_(f, e, v, #v)
#define generate_named_value(f, e, n, v)                   generate_named_value_(f, e, n, v, #v)
#define generate_value_from_string(f, e, s, n, v)          generate_value_from_string_(f, e, s, n, v, #v)
#define generate_upper_lower_from_string(f, e, s, n, u, l) generate_upper_lower_from_string_(f, e, s, n, u, l, #u, #l)
#define generate_upper_lower_from_string_div2(f, e, s, n, u, l) generate_upper_lower_from_string_div2_(f, e, s, n, u, l, #u, #l)

internal void
generate_words_row(FILE *fileOutput, u32 elemCount, u32 *value)
{
    for (u32 idx = 0; idx < elemCount; ++idx)
    {
        fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", value[idx]);
    }
}

internal void
generate_value_(FILE *fileOutput, u32 elemCount, u32 *value, char *valueName)
{
    fprintf(fileOutput, "global u32 %s[%u] = {", valueName, elemCount);
    generate_words_row(fileOutput, elemCount, value);
    fprintf(fileOutput, "};\n");
}

internal void
generate_named_value_(FILE *fileOutput, u32 elemCount, String name, u32 *value, char *valueName)
{
    fprintf(fileOutput, "// NOTE(generator): %.*s\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, value, valueName);
}

internal void
generate_value_from_string_(FILE *fileOutput, u32 elemCount, String strValue, String name, u32 *value, char *valueName)
{
    xf_from_string(elemCount, strValue, value);

    if (!value[elemCount - 1])
    {
        fprintf(stderr, "NOT ENOUGH DIGITS OF %.*s!!!\n", STR_FMT(name));
    }

    generate_named_value_(fileOutput, elemCount, name, value, valueName);
}

#if EXTENDED_PRECISION_FULL
internal void
generate_upper_lower_from_string_(FILE *fileOutput, u32 elemCount, String constName, String name, u32 *upper, u32 *lower, char *upperName, char *lowerName)
{
    // TODO(michiel): Do a test with different precisions (one word, two words ore more extra)
    // For example: 0.001 * lower + 0.001 * upper
    u32 extraCount = elemCount - (XFLOAT_MANTISSA_IDX + 1);
    u32 totalCount = elemCount + extraCount;
    u32 total[totalCount];

    xf_from_string(totalCount, constName, total);
    if (!total[totalCount - 1])
    {
        fprintf(stderr, "NOT ENOUGH DIGITS OF %.*s!!!\n", STR_FMT(name));
    }
    xf_copy(elemCount, total, upper);

    lower[0] = total[0];
    lower[1] = 0;
    xf_set_exponent(elemCount, lower, xf_get_exponent(elemCount, lower) - 32*extraCount);
    copy(extraCount * sizeof(u32), total + elemCount, lower + 2);

    fprintf(fileOutput, "// NOTE(generator): %.*s most-significant 32bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, upper, upperName);

    fprintf(fileOutput, "// NOTE(generator): %.*s least-significant bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, lower, lowerName);
}

internal void
generate_upper_lower_from_string_div2_(FILE *fileOutput, u32 elemCount, String constName, String name, u32 *upper, u32 *lower, char *upperName, char *lowerName)
{
    // TODO(michiel): Do a test with different precisions (one word, two words ore more extra)
    // For example: 0.001 * lower + 0.001 * upper
    u32 extraCount = elemCount - (XFLOAT_MANTISSA_IDX + 1);
    u32 totalCount = elemCount + extraCount;
    u32 total[totalCount];

    xf_from_string(totalCount, constName, total);
    if (!total[totalCount - 1])
    {
        fprintf(stderr, "NOT ENOUGH DIGITS OF %.*s!!!\n", STR_FMT(name));
    }
    xf_naive_div2(totalCount, total);

    xf_copy(elemCount, total, upper);

    lower[0] = total[0];
    lower[1] = 0;
    xf_set_exponent(elemCount, lower, xf_get_exponent(elemCount, lower) - 32*extraCount);
    copy(extraCount * sizeof(u32), total + elemCount, lower + 2);

    fprintf(fileOutput, "// NOTE(generator): %.*s most-significant 32bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, upper, upperName);

    fprintf(fileOutput, "// NOTE(generator): %.*s least-significant bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, lower, lowerName);
}
#else
internal void
generate_upper_lower_from_string_(FILE *fileOutput, u32 elemCount, String constName, String name, u32 *upper, u32 *lower, char *upperName, char *lowerName)
{
    // TODO(michiel): Do a test with different precisions (one word, two words ore more extra)
    // For example: 0.001 * lower + 0.001 * upper
    u32 total[elemCount + 1];

    xf_from_string(elemCount + 1, constName, total);
    if (!total[elemCount])
    {
        fprintf(stderr, "NOT ENOUGH DIGITS OF %.*s!!!\n", STR_FMT(name));
    }
    upper[0] = total[0];
    upper[2] = total[2];

    lower[0] = total[0];
    xf_set_exponent(elemCount, lower, xf_get_exponent(elemCount, lower) - 32);
    copy((elemCount - 2) * sizeof(u32), total + 3, lower + 2);

    fprintf(fileOutput, "// NOTE(generator): %.*s most-significant 32bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, upper, upperName);

    fprintf(fileOutput, "// NOTE(generator): %.*s least-significant bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, lower, lowerName);
}

internal void
generate_upper_lower_from_string_div2_(FILE *fileOutput, u32 elemCount, String constName, String name, u32 *upper, u32 *lower, char *upperName, char *lowerName)
{
    // TODO(michiel): Do a test with different precisions (one word, two words ore more extra)
    // For example: 0.001 * lower + 0.001 * upper
    u32 total[elemCount + 1];

    xf_from_string(elemCount + 1, constName, total);
    if (!total[elemCount])
    {
        fprintf(stderr, "NOT ENOUGH DIGITS OF %.*s!!!\n", STR_FMT(name));
    }
    xf_naive_div2(elemCount, total);

    upper[0] = total[0];
    upper[2] = total[2];

    lower[0] = total[0];
    xf_set_exponent(elemCount, lower, xf_get_exponent(elemCount, lower) - 32);
    copy((elemCount - 2) * sizeof(u32), total + 3, lower + 2);

    fprintf(fileOutput, "// NOTE(generator): %.*s most-significant 32bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, upper, upperName);

    fprintf(fileOutput, "// NOTE(generator): %.*s least-significant bits\n", STR_FMT(name));
    generate_value_(fileOutput, elemCount, lower, lowerName);
}
#endif

s32 main(s32 argc, char **argv)
{
    u32 elemCount = 16;

    if (argc > 1)
    {
        elemCount = number_from_string(string(string_length(argv[1]), argv[1]));
    }

    u32 *tenA = allocate_array(u32, elemCount, 0);
    u32 *tenB = allocate_array(u32, elemCount, 0);

    u8 outputFilenameBuf[128];
    String outputFilename = string_fmt(array_count(outputFilenameBuf), outputFilenameBuf,
                                       "xfloat_constants_%u.c", elemCount);
    FILE *fileOutput = fopen(to_cstring(outputFilename), "wb");

    // NOTE(michiel): Init a to 10
    xf_clear(elemCount, tenA);
    xf_set_exponent(elemCount, tenA, XFLOAT_EXP_BIAS + 4);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    // NOTE(michiel): Init b to 10
    xf_copy(elemCount, tenA, tenB);

    u32 genTens = 0;
    u32 prevExp = xf_get_exponent(elemCount, tenB);
    u32 nextExp = xf_get_exponent(elemCount, tenA);
    while ((prevExp <= nextExp) && (nextExp < XFLOAT_MAX_EXPONENT))
    {
        ++genTens;
        xf_mul(elemCount, tenA, tenB, tenA);
        xf_copy(elemCount, tenA, tenB);
        prevExp = nextExp;
        nextExp = xf_get_exponent(elemCount, tenA);
    }

    // NOTE(michiel): Print defines
    XFLOAT_NUMBER_TENS = genTens - 1;
    XFLOAT_MIN_NTEN = -(1 << XFLOAT_NUMBER_TENS);
    XFLOAT_MAX_NTEN = 1 << XFLOAT_NUMBER_TENS;
    fprintf(fileOutput, "#define XFLOAT_NUMBER_TENS  %10d\n", XFLOAT_NUMBER_TENS);
    fprintf(fileOutput, "#define XFLOAT_MIN_NTEN     %10d\n", XFLOAT_MIN_NTEN);
    fprintf(fileOutput, "#define XFLOAT_MAX_NTEN     %10d\n", XFLOAT_MAX_NTEN);
    fprintf(fileOutput, "\n");

    // NOTE(michiel): Init a to 10
    xf_clear(elemCount, tenA);
    xf_set_exponent(elemCount, tenA, XFLOAT_EXP_BIAS + 4);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    // NOTE(michiel): Init b to 1
    xf_clear(elemCount, tenB);
    xf_set_exponent(elemCount, tenB, XFLOAT_EXP_BIAS + 1);
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

    gXF_Tens = allocate_array(u32, genTens * elemCount, 0);
    fprintf(fileOutput, "global u32 gXF_Tens[%u][%u] = {\n", genTens, elemCount);

    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_mul(elemCount, tenA, tenB, tenA);
        xf_copy(elemCount, tenA, xf_get_power_of_ten(elemCount, tenIdx));

        fprintf(fileOutput, "    // NOTE(generator) 10^%u\n    {", 1 << (tenIdx));
        generate_words_row(fileOutput, elemCount, tenA);
        fprintf(fileOutput, "},\n");
        xf_copy(elemCount, tenA, tenB);
    }
    fprintf(fileOutput, "};\n\n");

    // NOTE(michiel): Init a to 10
    xf_clear(elemCount, tenA);
    xf_set_exponent(elemCount, tenA, XFLOAT_EXP_BIAS + 4);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    // NOTE(michiel): Init b to 1
    xf_clear(elemCount, tenB);
    xf_set_exponent(elemCount, tenB, XFLOAT_EXP_BIAS + 1);
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

    gXF_Tenths = allocate_array(u32, genTens * elemCount, 0);
    fprintf(fileOutput, "global u32 gXF_Tenths[%u][%u] = {\n", genTens, elemCount);
    xf_div(elemCount, tenB, tenA, tenA);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_copy(elemCount, tenA, xf_get_power_of_tenths(elemCount, tenIdx));

        fprintf(fileOutput, "    // NOTE(generator) 10^-%u\n    {", 1 << (tenIdx));
        generate_words_row(fileOutput, elemCount, tenA);
        fprintf(fileOutput, "},\n");
        xf_copy(elemCount, tenA, tenB);
        xf_mul(elemCount, tenA, tenB, tenA);
    }
    fprintf(fileOutput, "};\n\n");

    fprintf(fileOutput, "internal u32 *\nxf_get_power_of_ten(u32 elemCount, u32 index)\n");
    fprintf(fileOutput, "{\n    i_expect(index <= XFLOAT_NUMBER_TENS);\n    return &gXF_Tens[index][0];\n}\n\n");

    fprintf(fileOutput, "internal u32 *\nxf_get_power_of_tenths(u32 elemCount, u32 index)\n");
    fprintf(fileOutput, "{\n    i_expect(index <= XFLOAT_NUMBER_TENS);\n    return &gXF_Tenths[index][0];\n}\n\n");

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Print 0
    gXF_Zero = allocate_array(u32, elemCount, 0);
    generate_named_value(fileOutput, elemCount, stringc("0.0e0"), gXF_Zero);

    // NOTE(michiel): Print 1
    gXF_One = allocate_array(u32, elemCount, 0);
    xf_set_exponent(elemCount, gXF_One, XFLOAT_EXP_BIAS + 1);
    gXF_One[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;
    generate_named_value(fileOutput, elemCount, stringc("1.0e0"), gXF_One);

    // NOTE(michiel): Print 0.5
    gXF_Half = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_One, gXF_Half);
    xf_naive_div2(elemCount, gXF_Half);
    generate_named_value(fileOutput, elemCount, stringc("0.5e0"), gXF_Half);

    // NOTE(michiel): Print 2.0
    gXF_Two = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_One, gXF_Two);
    xf_naive_mul2(elemCount, gXF_Two);
    generate_named_value(fileOutput, elemCount, stringc("2.0e0"), gXF_Two);

    // NOTE(michiel): Print 3.0
    gXF_Three = allocate_array(u32, elemCount, 0);
    xf_add(elemCount, gXF_One, gXF_Two, gXF_Three);
    generate_named_value(fileOutput, elemCount, stringc("3.0e0"), gXF_Three);

    // NOTE(michiel): Print 4.0
    gXF_Four = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_Two, gXF_Four);
    xf_naive_mul2(elemCount, gXF_Four);
    generate_named_value(fileOutput, elemCount, stringc("4.0e0"), gXF_Four);

    // NOTE(michiel): Print 5.0
    gXF_Five = allocate_array(u32, elemCount, 0);
    xf_add(elemCount, gXF_Four, gXF_One, gXF_Five);
    generate_named_value(fileOutput, elemCount, stringc("5.0e0"), gXF_Five);

    // NOTE(michiel): Print 6.0
    gXF_Six = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_Three, gXF_Six);
    xf_naive_mul2(elemCount, gXF_Six);
    generate_named_value(fileOutput, elemCount, stringc("6.0e0"), gXF_Six);

    // NOTE(michiel): Print 7.0
    gXF_Seven = allocate_array(u32, elemCount, 0);
    xf_add(elemCount, gXF_Six, gXF_One, gXF_Seven);
    generate_named_value(fileOutput, elemCount, stringc("7.0e0"), gXF_Seven);

    // NOTE(michiel): Print 8.0
    gXF_Eight = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_Four, gXF_Eight);
    xf_naive_mul2(elemCount, gXF_Eight);
    generate_named_value(fileOutput, elemCount, stringc("8.0e0"), gXF_Eight);

    // NOTE(michiel): Print 9.0
    gXF_Nine = allocate_array(u32, elemCount, 0);
    xf_add(elemCount, gXF_Eight, gXF_One, gXF_Nine);
    generate_named_value(fileOutput, elemCount, stringc("9.0e0"), gXF_Nine);

    // NOTE(michiel): Print -2.0
    gXF_NegativeTwo = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_Two, gXF_NegativeTwo);
    xf_negate(elemCount, gXF_NegativeTwo);
    generate_named_value(fileOutput, elemCount, stringc("-2.0e0"), gXF_NegativeTwo);

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Constants from a string definition

    // NOTE(michiel): Print pi
    gXF_Pi = allocate_array(u32, elemCount, 0);
    generate_value_from_string(fileOutput, elemCount, gPiString, stringc("pi"), gXF_Pi);

    // NOTE(michiel): Print pi/2
    gXF_PiOver2 = allocate_array(u32, elemCount, 0);
    xf_copy(elemCount, gXF_Pi, gXF_PiOver2);
    xf_naive_div2(elemCount, gXF_PiOver2);
    generate_named_value(fileOutput, elemCount, stringc("pi/2"), gXF_PiOver2);

    // NOTE(michiel): Print sqrt(2)
    gXF_Sqrt2 = allocate_array(u32, elemCount, 0);
    generate_value_from_string(fileOutput, elemCount, gSqrt2String, stringc("sqrt(2)"), gXF_Sqrt2);

    // NOTE(michiel): Print log(2)
    gXF_Log2 = allocate_array(u32, elemCount, 0);
    generate_value_from_string(fileOutput, elemCount, gLog2String, stringc("log(2)"), gXF_Log2);

    // NOTE(michiel): Print log(10)
    gXF_Log10 = allocate_array(u32, elemCount, 0);
    generate_value_from_string(fileOutput, elemCount, gLog10String, stringc("log(10)"), gXF_Log10);

    // NOTE(michiel): Print log2(e)
    gXF_Log2e = allocate_array(u32, elemCount, 0);
    generate_value_from_string(fileOutput, elemCount, gLog2eString, stringc("log2(e)"), gXF_Log2e);

    // NOTE(michiel): Print log10(e)
    gXF_Log10e = allocate_array(u32, elemCount, 0);
    generate_value_from_string(fileOutput, elemCount, gLog10eString, stringc("log10(e)"), gXF_Log10e);

    // NOTE(michiel): Print tan(pi/8)
    // TODO(michiel): Or should this come from a string as well?
    // It should offer more precision as the minus one here will introduce a unknown bit at the lower end.
    gXF_TanPiOver8 = allocate_array(u32, elemCount, 0);
    xf_sub(elemCount, gXF_Sqrt2, gXF_One, gXF_TanPiOver8);
    generate_named_value(fileOutput, elemCount, stringc("tan(pi/8) = sqrt(2) - 1"), gXF_TanPiOver8);

    // NOTE(michiel): Print tan(3pi/8)
    gXF_Tan3PiOver8 = allocate_array(u32, elemCount, 0);
    xf_add(elemCount, gXF_Sqrt2, gXF_One, gXF_Tan3PiOver8);
    generate_named_value(fileOutput, elemCount, stringc("tan(3pi/8) = sqrt(2) + 1"), gXF_Tan3PiOver8);

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Coefficients for polynomial approximations

    fprintf(fileOutput, "// TODO(generator): Calculate these from scratch\n"
            "// These are polynomial coefficients that approximate the square root function\n"
            "// in the range of [0.5, 1.0].\n");
    gXF_SquareRootCoef0 = allocate_array(u32, elemCount, 0);
    {
        i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);
        xf_set_exponent(elemCount, gXF_SquareRootCoef0, XFLOAT_EXP_BIAS - 1);
        gXF_SquareRootCoef0[XFLOAT_MANTISSA_IDX + 1] = 0xA08BDC7D;
        gXF_SquareRootCoef0[XFLOAT_MANTISSA_IDX + 2] = 0xD5FFE300;
        generate_value(fileOutput, elemCount, gXF_SquareRootCoef0);
    }
    gXF_SquareRootCoef1 = allocate_array(u32, elemCount, 0);
    {
        i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);
        xf_set_exponent(elemCount, gXF_SquareRootCoef1, XFLOAT_EXP_BIAS);
        gXF_SquareRootCoef1[XFLOAT_MANTISSA_IDX + 1] = 0xE3E3C2AE;
        gXF_SquareRootCoef1[XFLOAT_MANTISSA_IDX + 2] = 0x4C146700;
        generate_value(fileOutput, elemCount, gXF_SquareRootCoef1);
    }
    gXF_SquareRootCoef2 = allocate_array(u32, elemCount, 0);
    {
        i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);
        xf_make_negative(elemCount, gXF_SquareRootCoef2);
        xf_set_exponent(elemCount, gXF_SquareRootCoef2, XFLOAT_EXP_BIAS - 2);
        gXF_SquareRootCoef2[XFLOAT_MANTISSA_IDX + 1] = 0xD14FC42F;
        gXF_SquareRootCoef2[XFLOAT_MANTISSA_IDX + 2] = 0xE79BA800;
        generate_value(fileOutput, elemCount, gXF_SquareRootCoef2);
    }

    fprintf(fileOutput, "\n");

    // NOTE(michiel): For the next part we need to expand the elemcount.

#if EXTENDED_PRECISION_FULL
    u32 expandedCount = elemCount + (elemCount - (XFLOAT_MANTISSA_IDX + 1));
#else
    u32 expandedCount = elemCount + 1;
#endif

    deallocate(tenA);
    deallocate(tenB);
    tenA = allocate_array(u32, expandedCount, 0);
    tenB = allocate_array(u32, expandedCount, 0);

    xf_clear(expandedCount, tenA);
    xf_clear(expandedCount, tenB);
    xf_set_exponent(expandedCount, tenA, XFLOAT_EXP_BIAS + 4);
    xf_set_exponent(expandedCount, tenB, XFLOAT_EXP_BIAS + 1);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

    deallocate(gXF_Tens);
    gXF_Tens = allocate_array(u32, genTens * expandedCount, 0);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_mul(expandedCount, tenA, tenB, tenA);
        xf_copy(expandedCount, tenA, xf_get_power_of_ten(expandedCount, tenIdx));
        xf_copy(expandedCount, tenA, tenB);
    }

    xf_clear(expandedCount, tenA);
    xf_clear(expandedCount, tenB);
    xf_set_exponent(expandedCount, tenA, XFLOAT_EXP_BIAS + 4);
    xf_set_exponent(expandedCount, tenB, XFLOAT_EXP_BIAS + 1);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

    deallocate(gXF_Tenths);
    gXF_Tenths = allocate_array(u32, genTens * expandedCount, 0);
    xf_div(expandedCount, tenB, tenA, tenA);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_copy(expandedCount, tenA, xf_get_power_of_tenths(expandedCount, tenIdx));
        xf_copy(expandedCount, tenA, tenB);
        xf_mul(expandedCount, tenA, tenB, tenA);
    }

    gXF_PiOver2Upper = allocate_array(u32, elemCount, 0);
    gXF_PiOver2Lower = allocate_array(u32, elemCount, 0);
    gXF_Log2Upper = allocate_array(u32, elemCount, 0);
    gXF_Log2Lower = allocate_array(u32, elemCount, 0);
    gXF_Log2eUpper = allocate_array(u32, elemCount, 0);
    gXF_Log2eLower = allocate_array(u32, elemCount, 0);
    gXF_Log10eUpper = allocate_array(u32, elemCount, 0);
    gXF_Log10eLower = allocate_array(u32, elemCount, 0);
    generate_upper_lower_from_string_div2(fileOutput, elemCount, gPiString, stringc("pi/2"), gXF_PiOver2Upper, gXF_PiOver2Lower);
    generate_upper_lower_from_string(fileOutput, elemCount, gLog2String, stringc("log(2)"), gXF_Log2Upper, gXF_Log2Lower);
    generate_upper_lower_from_string(fileOutput, elemCount, gLog2eString, stringc("log2(e)"), gXF_Log2eUpper, gXF_Log2eLower);
    generate_upper_lower_from_string(fileOutput, elemCount, gLog10eString, stringc("log10(e)"), gXF_Log10eUpper, gXF_Log10eLower);

    fprintf(fileOutput, "\n");
    fclose(fileOutput);
}


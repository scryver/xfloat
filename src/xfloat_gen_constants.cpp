#include "../libberdip/src/platform.h"

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
global u32 *gXF_Sqrt2;
global u32 *gXF_Pi;
global u32 *gXF_PiOver2;

global u32 *gXF_Log2;
global u32 *gXF_Log10;
global u32 *gXF_Log2e;
global u32 *gXF_Log10e;

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
#include "xfloat.cpp"
#include "xfloat_math.cpp"
#include "xfloat_string.cpp"

#include "xfloat_gen_strings.cpp"

// NOTE(michiel): This can generate the values needed to parse strings as xfloats and outputs them to a file.
// It will also parse in common values from xfloat_gen_strings

#define generate_value(f, e, v)                            generate_value_(f, e, v, #v)
#define generate_named_value(f, e, n, v)                   generate_named_value_(f, e, n, v, #v)
#define generate_value_from_string(f, e, s, n, v)          generate_value_from_string_(f, e, s, n, v, #v)
#define generate_upper_lower_from_string(f, e, s, n, u, l) generate_upper_lower_from_string_(f, e, s, n, u, l, #u, #l)

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

internal void
generate_upper_lower_from_string_(FILE *fileOutput, u32 elemCount, String constName, String name, u32 *upper, u32 *lower, char *upperName, char *lowerName)
{
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

s32 main(s32 argc, char **argv)
{
    u32 elemCount = 16;

    if (argc > 1)
    {
        elemCount = number_from_string(string(argv[1]));
    }

    u32 *tenA = allocate_array(u32, elemCount);
    u32 *tenB = allocate_array(u32, elemCount);

    u8 outputFilenameBuf[128];
    String outputFilename = string_fmt(array_count(outputFilenameBuf), outputFilenameBuf,
                                       "xfloat_constants_%u.cpp", elemCount);
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
        xf_multiply(elemCount, tenA, tenB, tenA);
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

    gXF_Tens = allocate_array(u32, genTens * elemCount);
    fprintf(fileOutput, "global u32 gXF_Tens[%u][%u] = {\n", genTens, elemCount);

    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_multiply(elemCount, tenA, tenB, tenA);
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

    gXF_Tenths = allocate_array(u32, genTens * elemCount);
    fprintf(fileOutput, "global u32 gXF_Tenths[%u][%u] = {\n", genTens, elemCount);
    xf_divide(elemCount, tenB, tenA, tenA);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_copy(elemCount, tenA, xf_get_power_of_tenths(elemCount, tenIdx));

        fprintf(fileOutput, "    // NOTE(generator) 10^-%u\n    {", 1 << (tenIdx));
        generate_words_row(fileOutput, elemCount, tenA);
        fprintf(fileOutput, "},\n");
        xf_copy(elemCount, tenA, tenB);
        xf_multiply(elemCount, tenA, tenB, tenA);
    }
    fprintf(fileOutput, "};\n\n");

    fprintf(fileOutput, "internal u32 *\nxf_get_power_of_ten(u32 elemCount, u32 index)\n");
    fprintf(fileOutput, "{\n    i_expect(index <= XFLOAT_NUMBER_TENS);\n    return &gXF_Tens[index][0];\n}\n\n");

    fprintf(fileOutput, "internal u32 *\nxf_get_power_of_tenths(u32 elemCount, u32 index)\n");
    fprintf(fileOutput, "{\n    i_expect(index <= XFLOAT_NUMBER_TENS);\n    return &gXF_Tenths[index][0];\n}\n\n");

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Print 0
    gXF_Zero = allocate_array(u32, elemCount);
    generate_named_value(fileOutput, elemCount, string("0.0e0"), gXF_Zero);

    // NOTE(michiel): Print 1
    gXF_One = allocate_array(u32, elemCount);
    xf_set_exponent(elemCount, gXF_One, XFLOAT_EXP_BIAS + 1);
    gXF_One[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;
    generate_named_value(fileOutput, elemCount, string("1.0e0"), gXF_One);

    // NOTE(michiel): Print 0.5
    gXF_Half = allocate_array(u32, elemCount);
    xf_copy(elemCount, gXF_One, gXF_Half);
    xf_naive_div2(elemCount, gXF_Half);
    generate_named_value(fileOutput, elemCount, string("0.5e0"), gXF_Half);

    // NOTE(michiel): Print 2.0
    gXF_Two = allocate_array(u32, elemCount);
    xf_copy(elemCount, gXF_One, gXF_Two);
    xf_naive_mul2(elemCount, gXF_Two);
    generate_named_value(fileOutput, elemCount, string("2.0e0"), gXF_Two);

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Constants from a string definition

    // NOTE(michiel): Print pi
    gXF_Pi = allocate_array(u32, elemCount);
    generate_value_from_string(fileOutput, elemCount, gPiString, string("Pi"), gXF_Pi);

    // NOTE(michiel): Print pi/2
    gXF_PiOver2 = allocate_array(u32, elemCount);
    xf_copy(elemCount, gXF_Pi, gXF_PiOver2);
    xf_naive_div2(elemCount, gXF_PiOver2);
    generate_named_value(fileOutput, elemCount, string("Pi / 2"), gXF_PiOver2);

    // NOTE(michiel): Print sqrt(2)
    gXF_Sqrt2 = allocate_array(u32, elemCount);
    generate_value_from_string(fileOutput, elemCount, gSqrt2String, string("Sqrt(2)"), gXF_Sqrt2);

    // NOTE(michiel): Print log(2)
    gXF_Log2 = allocate_array(u32, elemCount);
    generate_value_from_string(fileOutput, elemCount, gLog2String, string("Log(2)"), gXF_Log2);

    // NOTE(michiel): Print log(10)
    gXF_Log10 = allocate_array(u32, elemCount);
    generate_value_from_string(fileOutput, elemCount, gLog10String, string("Log(10)"), gXF_Log10);

    // NOTE(michiel): Print log2(e)
    gXF_Log2e = allocate_array(u32, elemCount);
    generate_value_from_string(fileOutput, elemCount, gLog2eString, string("Log2(e)"), gXF_Log2e);

    // NOTE(michiel): Print log10(e)
    gXF_Log10e = allocate_array(u32, elemCount);
    generate_value_from_string(fileOutput, elemCount, gLog10eString, string("Log10(e)"), gXF_Log10e);

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Coefficients for polynomial approximations

    fprintf(fileOutput, "// TODO(generator): Calculate these from scratch\n"
            "// These are polynomial coefficients that approximate the square root function\n"
            "// in the range of [0.5, 1.0].\n");
    gXF_SquareRootCoef0 = allocate_array(u32, elemCount);
    {
        i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);
        xf_set_exponent(elemCount, gXF_SquareRootCoef0, XFLOAT_EXP_BIAS - 1);
        gXF_SquareRootCoef0[XFLOAT_MANTISSA_IDX + 1] = 0xA08BDC7D;
        gXF_SquareRootCoef0[XFLOAT_MANTISSA_IDX + 2] = 0xD5FFE300;
        generate_value(fileOutput, elemCount, gXF_SquareRootCoef0);
    }
    gXF_SquareRootCoef1 = allocate_array(u32, elemCount);
    {
        i_expect(elemCount > XFLOAT_MANTISSA_IDX + 2);
        xf_set_exponent(elemCount, gXF_SquareRootCoef1, XFLOAT_EXP_BIAS);
        gXF_SquareRootCoef1[XFLOAT_MANTISSA_IDX + 1] = 0xE3E3C2AE;
        gXF_SquareRootCoef1[XFLOAT_MANTISSA_IDX + 2] = 0x4C146700;
        generate_value(fileOutput, elemCount, gXF_SquareRootCoef1);
    }
    gXF_SquareRootCoef2 = allocate_array(u32, elemCount);
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

    deallocate(tenA);
    deallocate(tenB);
    tenA = allocate_array(u32, elemCount + 1);
    tenB = allocate_array(u32, elemCount + 1);

    xf_clear(elemCount + 1, tenA);
    xf_clear(elemCount + 1, tenB);
    xf_set_exponent(elemCount + 1, tenA, XFLOAT_EXP_BIAS + 4);
    xf_set_exponent(elemCount + 1, tenB, XFLOAT_EXP_BIAS + 1);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

    deallocate(gXF_Tens);
    gXF_Tens = allocate_array(u32, genTens * (elemCount + 1));
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_multiply(elemCount + 1, tenA, tenB, tenA);
        xf_copy(elemCount + 1, tenA, xf_get_power_of_ten(elemCount + 1, tenIdx));
        xf_copy(elemCount + 1, tenA, tenB);
    }

    xf_clear(elemCount + 1, tenA);
    xf_clear(elemCount + 1, tenB);
    xf_set_exponent(elemCount + 1, tenA, XFLOAT_EXP_BIAS + 4);
    xf_set_exponent(elemCount + 1, tenB, XFLOAT_EXP_BIAS + 1);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

    deallocate(gXF_Tenths);
    gXF_Tenths = allocate_array(u32, genTens * (elemCount + 1));
    xf_divide(elemCount + 1, tenB, tenA, tenA);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_copy(elemCount + 1, tenA, xf_get_power_of_tenths(elemCount + 1, tenIdx));
        xf_copy(elemCount + 1, tenA, tenB);
        xf_multiply(elemCount + 1, tenA, tenB, tenA);
    }

    gXF_Log2Upper = allocate_array(u32, elemCount);
    gXF_Log2Lower = allocate_array(u32, elemCount);
    gXF_Log2eUpper = allocate_array(u32, elemCount);
    gXF_Log2eLower = allocate_array(u32, elemCount);
    gXF_Log10eUpper = allocate_array(u32, elemCount);
    gXF_Log10eLower = allocate_array(u32, elemCount);
    generate_upper_lower_from_string(fileOutput, elemCount, gLog2String, string("Log(2)"), gXF_Log2Upper, gXF_Log2Lower);
    generate_upper_lower_from_string(fileOutput, elemCount, gLog2eString, string("Log2(e)"), gXF_Log2eUpper, gXF_Log2eLower);
    generate_upper_lower_from_string(fileOutput, elemCount, gLog10eString, string("Log10(e)"), gXF_Log10eUpper, gXF_Log10eLower);

    fprintf(fileOutput, "\n");
    fclose(fileOutput);
}


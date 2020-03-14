#include "../libberdip/src/platform.h"

#include "xfloat.h"
#include "xfloat.cpp"

// NOTE(michiel): This can generate the values needed to parse strings as xfloats and prints them back

s32 main(s32 argc, char **argv)
{
    u32 elemCount = 16;
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
    fprintf(fileOutput, "#define XFLOAT_NUMBER_TENS  %10d\n", genTens - 1);
    fprintf(fileOutput, "#define XFLOAT_MIN_NTEN     %10d\n", -(1 << (genTens - 1)));
    fprintf(fileOutput, "#define XFLOAT_MAX_NTEN     %10d\n", 1 << (genTens - 1));
    fprintf(fileOutput, "\n");
    
    // NOTE(michiel): Print 0
    {
        u32 *zeroes = allocate_array(u32, elemCount);
        fprintf(fileOutput, "// NOTE(generator): 0.0e0\n"
                "global u32 gXF_Zero[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", zeroes[idx]);
        }
        fprintf(fileOutput, "};\n");
        deallocate(zeroes);
    }
    
    // NOTE(michiel): Print 1
    {
        u32 *ones = allocate_array(u32, elemCount);
        xf_set_exponent(elemCount, ones, XFLOAT_EXP_BIAS + 1);
        ones[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;
        
        fprintf(fileOutput, "// NOTE(generator): 1.0e0\n"
                "global u32 gXF_One[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", ones[idx]);
        }
        fprintf(fileOutput, "};\n");
        deallocate(ones);
    }
    
    // NOTE(michiel): Init a to 10
    xf_clear(elemCount, tenA);
    xf_set_exponent(elemCount, tenA, XFLOAT_EXP_BIAS + 4);
    tenA[XFLOAT_MANTISSA_IDX + 1] = 0xA0000000;
    // NOTE(michiel): Init b to 1
    xf_clear(elemCount, tenB);
    xf_set_exponent(elemCount, tenB, XFLOAT_EXP_BIAS + 1);
    tenB[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;
    
    fprintf(fileOutput, "global u32 gXF_Tens[%u][%u] = {\n", genTens, elemCount);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_multiply(elemCount, tenA, tenB, tenA);
        fprintf(fileOutput, "    // NOTE(generator) 10^%u\n    {", 1 << (tenIdx));
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", tenA[idx]);
        }
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
    
    fprintf(fileOutput, "global u32 gXF_Tenths[%u][%u] = {\n", genTens, elemCount);
    xf_divide(elemCount, tenB, tenA, tenA);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        fprintf(fileOutput, "    // NOTE(generator) 10^-%u\n    {", 1 << (tenIdx));
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", tenA[idx]);
        }
        fprintf(fileOutput, "},\n");
        xf_copy(elemCount, tenA, tenB);
        xf_multiply(elemCount, tenA, tenB, tenA);
    }
    fprintf(fileOutput, "};\n\n");
    fclose(fileOutput);
}


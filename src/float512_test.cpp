#include "../libberdip/src/platform.h"

#include "xfloat.h"
#include "xfloat.c"
#include "xfloat_constants_16.c"
#include "xfloat_string.c"

#include "float512.h"

s32 main(s32 argc, char **argv)
{
    f512 a = {};
    f512 b = F512(3.0f);

    u8 aBuf[512];
    u8 bBuf[512];
    String aStr;
    String bStr;

    fprintf(stdout, "Start:\n");

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Added:\n");
    a += b;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Multiply:\n");
    a *= b;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Subtract:\n");
    a = b - a;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Divide:\n");
    a /= b;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "-1/3:\n");
    a = F512(3.0f);
    b = F512(-1.0f);
    a = b / a;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "ans*3:\n");
    b = F512(3.0f);
    a *= b;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "3.8 - 3.0:\n");
    a = F512(3.8f);
    b = F512(3.0f);
    a = a - b;

    aStr = string_from_f512(&a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_f512(&b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    return 0;
}



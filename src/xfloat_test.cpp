#include "../libberdip/platform.h"

#include "xfloat.h"
#include "xfloat.cpp"
#include "xfloat_constants_16.cpp"
#include "xfloat_string.cpp"
#include "xfloat_math.cpp"

s32 main(s32 argc, char **argv)
{
    u32 elemCount = 16;
    u32 a[16];
    u32 b[16];
    xf_clear(elemCount, a);
    //xf_from_f64(elemCount, 2.5e-324, b);
    //xf_from_f32(elemCount, 0.8e-45f, b);
    xf_from_f32(elemCount, 3.0f, b);

    u8 aBuf[512];
    u8 bBuf[512];
    String aStr;
    String bStr;

    fprintf(stdout, "Start:\n");

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Added:\n");
    xf_add(elemCount, a, b, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Multiply:\n");
    xf_multiply(elemCount, a, b, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Subtract:\n");
    xf_subtract(elemCount, b, a, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "Divide:\n");
    xf_divide(elemCount, a, b, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "1/3:\n");
    xf_from_f32(elemCount, 3.0f, a);
    xf_from_f32(elemCount, 1.0f, b);
    xf_divide(elemCount, b, a, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "ans*3:\n");
    xf_from_f32(elemCount, 3.0f, b);
    xf_multiply(elemCount, a, b, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    fprintf(stdout, "1/(1/3):\n");
    xf_from_f32(elemCount, 3.0f, a);
    xf_from_f32(elemCount, 1.0f, b);
    xf_divide(elemCount, b, a, a);
    xf_divide(elemCount, b, a, a);

    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    bStr = string_from_xf(elemCount, b, U32_MAX, array_count(bBuf), bBuf);
    fprintf(stdout, "  A = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  B = %.*s\n", STR_FMT(bStr));

    String tenth = static_string("0.1");
    xf_from_string(elemCount, tenth, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String hundreth = static_string("0.01");
    xf_from_string(elemCount, hundreth, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String hundMicroth = static_string("0.0001");
    xf_from_string(elemCount, hundMicroth, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String tenNanoth = static_string("0.00000001");
    xf_from_string(elemCount, tenNanoth, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String tenthNext = static_string("1e-16");
    xf_from_string(elemCount, tenthNext, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String one = static_string("1");
    xf_from_string(elemCount, one, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String ten = static_string("10");
    xf_from_string(elemCount, ten, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String hundred = static_string("100");
    xf_from_string(elemCount, hundred, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String tenThousand = static_string("10000");
    xf_from_string(elemCount, tenThousand, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    String tenNext = static_string("1.0E8");
    xf_from_string(elemCount, tenNext, a);
    aStr = string_from_xf(elemCount, a, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  X = %.*s\n", STR_FMT(aStr));
    fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X}\n", a[0], a[1], a[2], a[3], a[4]);

    //
    // NOTE(michiel): TABLE GEN
    //
    u32 testNew[10] = {};
    u32 testNew2[10];

    testNew[0] = XFLOAT_EXP_BIAS + 4;
    testNew[2] = 0xA0000000;
    aStr = string_from_xf(elemCount, testNew, U32_MAX, array_count(aBuf), aBuf);
    //fprintf(stdout, "  10E1 = %.*s\n", STR_FMT(aStr));
    //fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X}\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);
    fprintf(stdout, "    {0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X},\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);

    xf_copy(10, testNew, testNew2);
    xf_multiply(10, testNew, testNew2, testNew);
    aStr = string_from_xf(elemCount, testNew, U32_MAX, array_count(aBuf), aBuf);
    //fprintf(stdout, "  10E2 = %.*s\n", STR_FMT(aStr));
    //fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X}\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);
    fprintf(stdout, "    {0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X},\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);

    xf_copy(10, testNew, testNew2);
    xf_multiply(10, testNew, testNew2, testNew);
    aStr = string_from_xf(elemCount, testNew, U32_MAX, array_count(aBuf), aBuf);
    //fprintf(stdout, "  10E4 = %.*s\n", STR_FMT(aStr));
    //fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X}\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);
    fprintf(stdout, "    {0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X},\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);

    xf_copy(10, testNew, testNew2);
    xf_multiply(10, testNew, testNew2, testNew);
    aStr = string_from_xf(elemCount, testNew, U32_MAX, array_count(aBuf), aBuf);
    //fprintf(stdout, "  10E8 = %.*s\n", STR_FMT(aStr));
    //fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X}\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);
    fprintf(stdout, "    {0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X},\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);

    xf_copy(10, testNew, testNew2);
    xf_multiply(10, testNew, testNew2, testNew);
    aStr = string_from_xf(elemCount, testNew, U32_MAX, array_count(aBuf), aBuf);
    //fprintf(stdout, "  10E16 = %.*s\n", STR_FMT(aStr));
    //fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X}\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);
    fprintf(stdout, "    {0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X},\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);

    xf_copy(10, testNew, testNew2);
    xf_multiply(10, testNew, testNew2, testNew);
    aStr = string_from_xf(elemCount, testNew, U32_MAX, array_count(aBuf), aBuf);
    //fprintf(stdout, "  10E32 = %.*s\n", STR_FMT(aStr));
    //fprintf(stdout, "  {%08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X, %08X}\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);
    fprintf(stdout, "    {0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X, 0x%08X},\n", testNew[0], testNew[1], testNew[2], testNew[3], testNew[4], testNew[5], testNew[6], testNew[7], testNew[8], testNew[9]);

    i_expect(elemCount == 16);

    u32 pi[16];
    xf_from_string(elemCount,
                   static_string("3.14159265358979323846264338327950288419716939937510582097494459230781640"
                                 "6286208998628034825342117067982148086513282306647093844609550582231725359"
                                 "4081284811174502841027019385211055596446229489549303819644288109756659334"
                                 "4612847564823378678316527120190914564856692346034861045432664821339360726"
                                 "0249141273724587006606315588174881520920962829254091715364367892590360011"
                                 "3305305488204665213841469519415116094330572703657595919530921861173819326"
                                 "1179310511854807446237996274956735188575272489122793818301194912983367336"
                                 "2440656643086021394946395224737190702179860943702770539217176293176752384"
                                 "6748184676694051320005681271452635608277857713427577896091736371787214684"
                                 "4090122495343014654958537105079227968925892354201995611212902196086403441"
                                 "8159813629774771309960518707211349999998372978049951059731732816096318595"
                                 "0244594553469083026425223082533446850352619311881710100031378387528865875"
                                 "3320838142061717766914730359825349042875546873115956286388235378759375195"
                                 "7781857780532171226806613001927876611195909216420198938095257201065485863"
                                 "2788659361533818279682303019520353018529689957736225994138912497217752834"
                                 "7913151557485724245415069595082953311686172785588907509838175463746493931"
                                 "9255060400927701671139009848824012858361603563707660104710181942955596198"
                                 "9467678374494482553797747268471040475346462080466842590694912933136770289"
                                 "8915210475216205696602405803815019351125338243003558764024749647326391419"
                                 "9272604269922796782354781636009341721641219924586315030286182974555706749"
                                 "8385054945885869269956909272107975093029553211653449872027559602364806654"
                                 "9911988183479775356636980742654252786255181841757467289097777279380008164"
                                 "7060016145249192173217214772350141441973568548161361157352552133475741849"
                                 "4684385233239073941433345477624168625189835694855620992192221842725502542"
                                 "5688767179049460165346680498862723279178608578438382796797668145410095388"
                                 "3786360950680064225125205117392984896084128488626945604241965285022210661"
                                 "1863067442786220391949450471237137869609563643719172874677646575739624138"
                                 "9086583264599581339047802759009946576407895126946839835259570982582262052"
                                 "2489407726719478268482601476990902640136394437455305068203496252451749399"
                                 "6514314298091906592509372216964615157098583874105978859597729754989301617"
                                 "5392846813826868386894277415599185592524595395943104997252468084598727364"
                                 "4695848653836736222626099124608051243884390451244136549762780797715691435"
                                 "9977001296160894416948685558484063534220722258284886481584560285060168427"
                                 "3945226746767889525213852254995466672782398645659611635488623057745649803"
                                 "5593634568174324112515076069479451096596094025228879710893145669136867228"
                                 "7489405601015033086179286809208747609178249385890097149096759852613655497"
                                 "8189312978482168299894872265880485756401427047755513237964145152374623436"
                                 "4542858444795265867821051141354735739523113427166102135969536231442952484"
                                 "9371871101457654035902799344037420073105785390621983874478084784896833214"
                                 "4571386875194350643021845319104848100537061468067491927819119793995206141"
                                 "9663428754440643745123718192179998391015919561814675142691239748940907186"
                                 "4942319615679452080951465502252316038819301420937621378559566389377870830"), pi);
    aStr = string_from_xf(elemCount, pi, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  PI   = %.*s\n", STR_FMT(aStr));

    u32 zeros[16];
    xf_from_string(elemCount, static_string("0e12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0e+12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0e-12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+0e-12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("-0e-12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, " -ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0.0E12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+0.0E12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+0.0E-12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+0.0E+12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("-0.0E-12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, " -ZERO = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("-0.0E+12"), zeros);
    aStr = string_from_xf(elemCount, zeros, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, " -ZERO = %.*s\n", STR_FMT(aStr));

    u32 ones[16];
    xf_from_string(elemCount, static_string("1"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("1.0"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("1.0e0"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0.000000000001e12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0.000000000001e+12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("1000000000000e-12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+1000000000000e-12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("-1000000000000e-12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, " -ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0.000000000001E12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+0.000000000001E+12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+1000000000000.0E-12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("+0.000000000001E+12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("-1000000000000.0E-12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, " -ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("-0.000000000001E+12"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, " -ONE  = %.*s\n", STR_FMT(aStr));

    xf_from_string(elemCount, static_string("0x0000.8p1"), ones);
    aStr = string_from_xf(elemCount, ones, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  ONE  = %.*s\n", STR_FMT(aStr));

#if 0
    u32 floorTest[16];

    u32 testFloors = 41;
    f32 testFloorStart = -2.0f;
    f32 testFloorEnd = 2.0f;

    for (u32 testIdx = 0; testIdx < testFloors; ++testIdx)
    {
        f32 testF = map((f32)testIdx, 0, testFloors - 1, testFloorStart, testFloorEnd);
        xf_from_f32(elemCount, testF, floorTest);
        xf_floor(elemCount, floorTest, floorTest);
        aStr = string_from_xf(elemCount, floorTest, 5, array_count(aBuf), aBuf);
        fprintf(stdout, " FLOOR(% 5.2f) = %.*s\n", testF, STR_FMT(aStr));
    }

    for (u32 testIdx = 0; testIdx < testFloors; ++testIdx)
    {
        f32 testF = map((f32)testIdx, 0, testFloors - 1, testFloorStart, testFloorEnd);
        xf_from_f32(elemCount, testF, floorTest);
        xf_round(elemCount, floorTest, floorTest);
        aStr = string_from_xf(elemCount, floorTest, 5, array_count(aBuf), aBuf);
        fprintf(stdout, " ROUND(% 5.2f) = %.*s\n", testF, STR_FMT(aStr));
    }
#endif

    u32 constOne[16];
    u32 constTwo[16];
    u32 constThree[16];
    u32 constFour[16];
    u32 constFive[16];
    u32 constSix[16];
    u32 constSeven[16];
    u32 constEight[16];
    u32 constNine[16];
    u32 constTen[16];
    u32 constEleven[16];
    u32 summer[16];
    xf_from_f32(elemCount, 1.0f, constOne);
    xf_from_f32(elemCount, 2.0f, constTwo);
    xf_from_f32(elemCount, 3.0f, constThree);
    xf_from_f32(elemCount, 4.0f, constFour);
    xf_from_f32(elemCount, 5.0f, constFive);
    xf_from_f32(elemCount, 6.0f, constSix);
    xf_from_f32(elemCount, 7.0f, constSeven);
    xf_from_f32(elemCount, 8.0f, constEight);
    xf_from_f32(elemCount, 9.0f, constNine);
    xf_from_f32(elemCount, 10.0f, constTen);
    xf_from_f32(elemCount, 11.0f, constEleven);

    xf_add(elemCount, constOne, constTwo, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 2 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constThree, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 3 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constFour, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 4 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constFive, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 5 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constSix, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 6 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constSeven, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 7 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constEight, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 8 = %.*s (0x%08X)\n", STR_FMT(aStr), summer[0]);

    xf_add(elemCount, constOne, constNine, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 9 = %.*s (0x%08X)\n", STR_FMT(aStr), summer[0]);

    xf_add(elemCount, constOne, constTen, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 10 = %.*s\n", STR_FMT(aStr));

    xf_add(elemCount, constOne, constEleven, summer);
    aStr = string_from_xf(elemCount, summer, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  1 + 11 = %.*s\n", STR_FMT(aStr));

    u32 sqrtTest[16];
    xf_from_f32(elemCount, 1000.0f, sqrtTest);
    xf_square_root(elemCount, sqrtTest, sqrtTest);
    aStr = string_from_xf(elemCount, sqrtTest, U32_MAX, array_count(aBuf), aBuf);
    fprintf(stdout, "  SQRT(3.0) = %.*s\n", STR_FMT(aStr));

    return 0;
}


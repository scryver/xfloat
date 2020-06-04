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

global u32 *gXF_Zero;
global u32 *gXF_One;
global u32 *gXF_Half;
global u32 *gXF_Two;
global u32 *gXF_Sqrt2;
global u32 *gXF_Pi;
global u32 *gXF_PiOver2;

global u32 *gXF_Log2Upper;
global u32 *gXF_Log2Lower;

#include "xfloat.h"
#include "xfloat.cpp"
#include "xfloat_math.cpp"
#include "xfloat_string.cpp"

// NOTE(michiel): This can generate the values needed to parse strings as xfloats and prints them back

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

    gXF_Tenths = allocate_array(u32, genTens * elemCount);
    fprintf(fileOutput, "global u32 gXF_Tenths[%u][%u] = {\n", genTens, elemCount);
    xf_divide(elemCount, tenB, tenA, tenA);
    for (u32 tenIdx = 0; tenIdx < genTens; ++tenIdx)
    {
        xf_copy(elemCount, tenA, xf_get_power_of_tenths(elemCount, tenIdx));

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

    fprintf(fileOutput, "internal u32 *\nxf_get_power_of_ten(u32 elemCount, u32 index)\n");
    fprintf(fileOutput, "{\n    i_expect(index <= XFLOAT_NUMBER_TENS);\n    return &gXF_Tens[index][0];\n}\n\n");

    fprintf(fileOutput, "internal u32 *\nxf_get_power_of_tenths(u32 elemCount, u32 index)\n");
    fprintf(fileOutput, "{\n    i_expect(index <= XFLOAT_NUMBER_TENS);\n    return &gXF_Tenths[index][0];\n}\n\n");

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Print 0
    gXF_Zero = allocate_array(u32, elemCount);
    {
        fprintf(fileOutput, "// NOTE(generator): 0.0e0\n"
                "global u32 gXF_Zero[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Zero[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

    // NOTE(michiel): Print 1
    gXF_One = allocate_array(u32, elemCount);
    {
        xf_set_exponent(elemCount, gXF_One, XFLOAT_EXP_BIAS + 1);
        gXF_One[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

        fprintf(fileOutput, "// NOTE(generator): 1.0e0\n"
                "global u32 gXF_One[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_One[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

    // NOTE(michiel): Print 0.5
    gXF_Half = allocate_array(u32, elemCount);
    {
        xf_set_exponent(elemCount, gXF_Half, XFLOAT_EXP_BIAS);
        gXF_Half[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

        fprintf(fileOutput, "// NOTE(generator): 0.5e0\n"
                "global u32 gXF_Half[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Half[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

    // NOTE(michiel): Print 2.0
    gXF_Two = allocate_array(u32, elemCount);
    {
        xf_set_exponent(elemCount, gXF_Two, XFLOAT_EXP_BIAS + 2);
        gXF_Two[XFLOAT_MANTISSA_IDX + 1] = 0x80000000;

        fprintf(fileOutput, "// NOTE(generator): 2.0e0\n"
                "global u32 gXF_Two[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Two[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

    fprintf(fileOutput, "\n");

    // NOTE(michiel): Constants from a string definition

    gXF_Pi = allocate_array(u32, elemCount);
    {
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
                                     "4942319615679452080951465502252316038819301420937621378559566389377870830"), gXF_Pi);

        if (!gXF_Pi[elemCount - 1])
        {
            fprintf(stderr, "NOT ENOUGH DIGITS OF PI!!!\n");
        }

        fprintf(fileOutput, "// NOTE(generator): Pi\n"
                "global u32 gXF_Pi[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Pi[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

    gXF_PiOver2 = allocate_array(u32, elemCount);
    {
        xf_copy(elemCount, gXF_Pi, gXF_PiOver2);
        xf_naive_div2(elemCount, gXF_PiOver2);
        fprintf(fileOutput, "// NOTE(generator): Pi / 2\n"
                "global u32 gXF_PiOver2[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_PiOver2[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

    gXF_Sqrt2 = allocate_array(u32, elemCount);
    {
        xf_from_string(elemCount,
                       static_string("1.41421356237309504880168872420969807856967187537694807317667973799073247"
                                     "8462107038850387534327641572735013846230912297024924836055850737212644121"
                                     "4970999358314132226659275055927557999505011527820605714701095599716059702"
                                     "7453459686201472851741864088919860955232923048430871432145083976260362799"
                                     "5251407989687253396546331808829640620615258352395054745750287759961729835"
                                     "5752203375318570113543746034084988471603868999706990048150305440277903164"
                                     "5424782306849293691862158057846311159666871301301561856898723723528850926"
                                     "4861249497715421833420428568606014682472077143585487415565706967765372022"
                                     "6485447015858801620758474922657226002085584466521458398893944370926591800"
                                     "3113882464681570826301005948587040031864803421948972782906410450726368813"
                                     "1373985525611732204024509122770022694112757362728049573810896750401836986"
                                     "8368450725799364729060762996941380475654823728997180326802474420629269124"
                                     "8590521810044598421505911202494413417285314781058036033710773091828693147"
                                     "1017111168391658172688941975871658215212822951848847208969463386289156288"
                                     "2765952635140542267653239694617511291602408715510135150455381287560052631"
                                     "4680171274026539694702403005174953188629256313851881634780015693691768818"
                                     "5237868405228783762938921430065586956868596459515550164472450983689603688"
                                     "7323114389415576651040883914292338113206052433629485317049915771756228549"
                                     "7414389991880217624309652065642118273167262575395947172559346372386322614"
                                     "8274262220867115583959992652117625269891754098815934864008345708518147223"
                                     "1814204070426509056532333398436457865796796519267292399875366617215982578"
                                     "8602633636178274959942194037777536814262177387991945513972312740668983299"
                                     "8989538672882285637869774966251996658352577619893932284534473569479496295"
                                     "2168891485492538904755828834526096524096542889394538646625744927556381964"
                                     "4103169798330618520193793849400571563337205480685405758679996701213722394"
                                     "7582142630658513221740883238294728761739364746783743196000159218880734785"
                                     "7617252211867490424977366929207311096369721608933708661156734585334833295"
                                     "2546758516447107578486024636008344491148185876555542864551233142199263113"
                                     "3251797060843655970435285641008791850076036100915946567067688360557174007"
                                     "6756905096136719401324935605240185999105062108163597726431380605467010293"
                                     "5699710424251057817495310572559349844511269227803449135066375687477602831"
                                     "6282960553242242695753452902883876844642917328277088831808702533985233812"
                                     "2749990812371892540726475367850304821591801886167108972869229201197599880"
                                     "7038185433325364602110822992792930728717807998880991767417741089830608003"
                                     "2631181642798823117154363869661702999934161614878686018045505553986913115"
                                     "1860103863753250045581860448040750241195184305674533683613674597374423988"
                                     "5532851793089603738989151731958741344288178421250219169518755934443873961"
                                     "8931454999990610758704909026088351763622474975785885836803745793115733980"
                                     "2099986622186949922595913276423619410592100328026149874566599688874067956"
                                     "1673918595728886424734635858868644968223860069833526427990562831656139139"
                                     "4255764906206518602164726303336297507569787060660685649816009271870929215"
                                     "3132368281356988937097416504474590960537472796524477094099241238710614470"
                                     "5439867436473384774548191008728862221495895295911878921491798339810837882"
                                     "7815306556231581036064867587303601450227320882935134138722768417667843690"
                                     "5294286984908384557445794095986260742499549168028530773989382960362133539"
                                     "8753205091998936075139064444957684569934712763645071632791547015977335486"
                                     "3893942325727754003826027478567417258095141630715959784981800944356037939"
                                     "0985590168272154034581581521004936662953448827107292396602321638238266612"
                                     "6268305025727811694510353793715688233659322978231929860646797898640920856"), gXF_Sqrt2);

        if (!gXF_Sqrt2[elemCount - 1])
        {
            fprintf(stderr, "NOT ENOUGH DIGITS OF SQRT(2)!!!\n");
        }

        fprintf(fileOutput, "// NOTE(generator): Sqrt(2)\n"
                "global u32 gXF_Sqrt2[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Sqrt2[idx]);
        }
        fprintf(fileOutput, "};\n");
    }

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
    {
        u32 *log2total = allocate_array(u32, elemCount + 1);
        xf_from_string(elemCount + 1,
                       static_string("0.69314718055994530941723212145817656807550013436025525412068000949339362"
                                     "1969694715605863326996418687542001481020570685733685520235758130557032670"
                                     "7516350759619307275708283714351903070386238916734711233501153644979552391"
                                     "2047517268157493206515552473413952588295045300709532636664265410423915781"
                                     "4952043740430385500801944170641671518644712839968171784546957026271631064"
                                     "5461502572074024816377733896385506952606683411372738737229289564935470257"
                                     "6265209885969320196505855476470330679365443254763274495125040606943814710"
                                     "4689946506220167720424524529612687946546193165174681392672504103802546259"
                                     "6568691441928716082938031727143677826548775664850856740776484514644399404"
                                     "6142260319309673540257444607030809608504748663852313818167675143866747664"
                                     "7890881437141985494231519973548803751658612753529166100071053558249879414"
                                     "7295092931138971559982056543928717000721808576102523688921324497138932037"
                                     "8439353088774825970171559107088236836275898425891853530243634214367061189"
                                     "2367891923723146723217205340164925687274778234453534764811494186423867767"
                                     "7440606956265737960086707625719918473402265146283790488306203306114463007"
                                     "3719489002743643965002580936519443041191150608094879306786515887090060520"
                                     "3468429736193841289652556539686022194122924207574321757489097706752687115"
                                     "8170511370091589426654785959648906530584602586683829400228330053820740056"
                                     "7705304678700184162404418833232798386349001563121889560650553151272199398"
                                     "3320307514084260914790012651682434438935724727882054862715527418772430024"
                                     "8979454019618723398086083166481149093066751933931289043164137068139777649"
                                     "8176974868903887789991296503619270710889264105230924783917373501229842420"
                                     "4995689359922066022046549415106139187885744245577510206837030866619480896"
                                     "4121868077902081815885800016881159730561866761991873952007667192145922367"
                                     "2060253959543654165531129517598994005600036651356756905124592682574394648"
                                     "3168332624901803824240824231452306140963805700702551387702681785163069025"
                                     "5137032340538021450190153740295099422629957796474271381573638017298739407"
                                     "0424217997226696297993931270693574724049338653087975872169964512944649188"
                                     "3771156701678598804981838896784134938314014073166472765327635919233511233"
                                     "3893387095132090592721854713289754707978913844454666761927028855334234298"
                                     "9932180376915497334026754675887323677834291619181043011609169526554785973"
                                     "2891763545556742863877463987101912431754255888301206779210280341206879759"
                                     "1430812833072303008834947057924965910058600123415617574132724659430684354"
                                     "6521113502154434153995538185652275022142456644000627618330320647272572197"
                                     "5152908278568421320795988638967277119552218819046603957009774706512619505"
                                     "2789322960889314056254334425523920620303439417773579455921259019925591148"
                                     "4402423901255425900312953705192206150643458378787300203541442178575801323"
                                     "6451660709914383145004985896688577222148652882169418127048860758972203216"
                                     "6631283783291567630749872985746389282693735098407780493950049339987626475"
                                     "5070316221613903484529942491724837340613662263834936811168416705692521475"
                                     "1383930638455371862687797328895558871634429756244755392366369488877823890"
                                     "1749810273565524050518547730619440524232212559024833082778888890596291197"
                                     "2995457441562451248592683112607467972816380902500056559991461283325435811"
                                     "1404848206064082422479240385576476235031100324259709142501114615584830670"
                                     "0125831821915347207474111940098355732728261442738213970704779562596705790"
                                     "2303384806171345555368553758106574973444792251119654616182789601006851296"
                                     "5395479658663783522473624546093585036050678414391144523145778033591792112"
                                     "7955705055554514387888188153519485934467246429498640506265184244753956637"
                                     "8337348220753329448130649336035461010177464932678771671986120739683201235"
                                     "960772902468304594031305637763132401080420285435902694509403074001493395"), log2total);
        if (!log2total[elemCount])
        {
            fprintf(stderr, "NOT ENOUGH DIGITS OF LOG2!!!\n");
        }
        gXF_Log2Upper[0] = log2total[0];
        gXF_Log2Upper[2] = log2total[2];

        gXF_Log2Lower[0] = log2total[0];
        xf_set_exponent(elemCount, gXF_Log2Lower, xf_get_exponent(elemCount, gXF_Log2Lower) - 32);
        copy((elemCount - 2) * sizeof(u32), log2total + 3, gXF_Log2Lower + 2);

        fprintf(fileOutput, "// NOTE(generator): Log2 most-significant 32bits\n"
                "global u32 gXF_Log2Upper[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Log2Upper[idx]);
        }
        fprintf(fileOutput, "};\n");
        fprintf(fileOutput, "// NOTE(generator): Log2 least-significant bits\n"
                "global u32 gXF_Log2Lower[%u] = {", elemCount);
        for (u32 idx = 0; idx < elemCount; ++idx)
        {
            fprintf(fileOutput, "%s0x%08X", idx > 0 ? ", " : "", gXF_Log2Lower[idx]);
        }
        fprintf(fileOutput, "};\n");

        deallocate(log2total);
    }

    fprintf(fileOutput, "\n");
    fclose(fileOutput);
}


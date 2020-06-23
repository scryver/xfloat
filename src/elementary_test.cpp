struct Environment
{
    // NOTE(michiel): iBeta is presumed to be 2
    u32 iT;
    b32 round;

    s32 minExp; // NOTE(michiel): Unbiased!
    s32 maxExp;

    u32 *eps;
    u32 *epsNeg;
    u32 *xMin;
    u32 *xMax;
};

internal void
init_environment(Environment *env, u32 elemCount, Arena *arena)
{
    env->iT = XFLOAT_MAX_BITS(elemCount);
    env->eps = arena_allocate_array(arena, u32, elemCount);
    env->epsNeg = arena_allocate_array(arena, u32, elemCount);
    env->xMin = arena_allocate_array(arena, u32, elemCount);
    env->xMax = arena_allocate_array(arena, u32, elemCount);

    env->minExp = 1 - (s32)XFLOAT_EXP_BIAS;
    env->maxExp = (s32)XFLOAT_MAX_EXPONENT - (s32)XFLOAT_EXP_BIAS - 1;

#if 1
    xf_copy(elemCount, gXF_One, env->eps);
    xf_set_exponent(elemCount, env->eps, XFLOAT_EXP_BIAS - (env->iT - 1));
    xf_copy(elemCount, gXF_One, env->epsNeg);
    xf_set_exponent(elemCount, env->epsNeg, XFLOAT_EXP_BIAS - (env->iT));
    //xf_print_raw(elemCount, env->eps);
    //xf_print_raw(elemCount, env->epsNeg);
#else
    xf_copy(elemCount, gXF_One, env->eps);
    xf_naive_div2(elemCount, env->eps);

    u32 accum[elemCount];
    while (1)
    {
        xf_add(elemCount, gXF_One, env->eps, accum);
        s32 compare = xf_compare(elemCount, gXF_One, accum);
        if (compare == 0)
        {
            // NOTE(michiel): Found it, back up one
            xf_naive_mul2(elemCount, env->eps);
            break;
        }
        else if (compare > 0)
        {
            fprintf(stderr, "WHAT?? We passed it??\n");
            xf_naive_mul2(elemCount, env->eps);
        }
        else
        {
            xf_naive_div2(elemCount, env->eps);
        }
    }
    //xf_print_raw(elemCount, env->eps);

    xf_copy(elemCount, gXF_One, env->epsNeg);
    xf_naive_div2(elemCount, env->epsNeg);

    while (1)
    {
        xf_subtract(elemCount, gXF_One, env->epsNeg, accum);
        s32 compare = xf_compare(elemCount, gXF_One, accum);
        if (compare == 0)
        {
            // NOTE(michiel): Found it, back up one
            xf_naive_mul2(elemCount, env->epsNeg);
            break;
        }
        else if (compare < 0)
        {
            fprintf(stderr, "WHAT?? We passed it??\n");
            xf_naive_mul2(elemCount, env->epsNeg);
        }
        else
        {
            xf_naive_div2(elemCount, env->epsNeg);
        }
    }
    //xf_print_raw(elemCount, env->epsNeg);
#endif

    xf_copy(elemCount, gXF_One, env->xMin);
    xf_set_exponent(elemCount, env->xMin, 1);

    xf_max_value(elemCount, env->xMax);

    u32 a[elemCount];
    u32 test[elemCount];
    xf_copy(elemCount, gXF_One, a);
    do
    {
        xf_add(elemCount, a, a, a);
        xf_add(elemCount, a, gXF_One, test);
        xf_sub(elemCount, test, a, test);
        xf_sub(elemCount, test, gXF_One, test);
    } while (xf_compare(elemCount, test, gXF_Zero) == 0);

    xf_sub(elemCount, gXF_Two, gXF_One, test);
    xf_add(elemCount, a, test, test);
    xf_sub(elemCount, test, a, test);
    env->round = xf_compare(elemCount, test, gXF_Zero) != 0;
}

internal void
test_square_root(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_square_root: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 465467867319LL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 SQBETA[elemCount];
    xf_copy(elemCount, gXF_Sqrt2, SQBETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    u32 A[elemCount];
    xf_div(elemCount, gXF_One, SQBETA, A);
    u32 B[elemCount];
    xf_copy(elemCount, gXF_One, B);

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    // NOTE(michiel): Random argument accuracy tests
    u32 C[elemCount];
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];

    u32 X[elemCount];
    u32 Y[elemCount];
    u32 Z[elemCount];
    u32 W[elemCount];

    for (u32 j = 0; j < 2; ++j)
    {
        xf_div(elemCount, B, A, C);
        u32 K1 = 0;
        u32 K3 = 0;
        xf_copy(elemCount, gXF_Zero, X1);
        xf_copy(elemCount, gXF_Zero, R6);
        xf_copy(elemCount, gXF_Zero, R7);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_log(series, elemCount, C, X);
            xf_mul(elemCount, A, X, X);
            xf_mul(elemCount, X, X, Y);

            xf_square_root(elemCount, Y, Z);

            xf_sub(elemCount, Z, X, W);
            xf_div(elemCount, W, X, W);

            s32 compare0 = xf_compare(elemCount, W, gXF_Zero);
            if (compare0 > 0) // W > 0
            {
                ++K1;
            }
            if (compare0 < 0) // W < 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);
            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
        }

        u32 K2 = N - K1 - K3;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        fprintf(stdout, "\nTest of square_root(X*X) - X\n");
        fprintf(stdout, "%u random arguments were tested from the interval (", N);
        xf_print(elemCount, A, 4);
        fprintf(stdout, ", ");
        xf_print(elemCount, B, 4);
        fprintf(stdout, ")\n");
        fprintf(stdout, "square_root(X) was larger %u times, agreed %u times and was smaller %u times.\n", K1, K2, K3);
        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_copy(elemCount, gXF_One, A);
        xf_copy(elemCount, SQBETA, B);
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nTest of special arguments\n");
    xf_copy(elemCount, env->xMin, X);
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root(xmin) = square_root(");
    xf_print(elemCount, X, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_sub(elemCount, gXF_One, env->epsNeg, X);
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root(1-epsNeg) = square_root(1-");
    xf_print(elemCount, env->epsNeg, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_copy(elemCount, gXF_One, X);
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root(1.0) = square_root(");
    xf_print(elemCount, X, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_add(elemCount, gXF_One, env->eps, X);
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root(1+eps) = square_root(1+");
    xf_print(elemCount, env->eps, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_copy(elemCount, env->xMax, X);
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root(xmax) = square_root(");
    xf_print(elemCount, X, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of error returns\n");
    xf_copy(elemCount, gXF_Zero, X);
    fprintf(stdout, "square_root() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n");
    // This should not trigger an error message
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_copy(elemCount, gXF_One, X);
    xf_make_negative(elemCount, X);
    fprintf(stdout, "square_root() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n");
    // This should trigger an error message
    xf_square_root(elemCount, X, Y);
    fprintf(stdout, "square_root returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of square_root\n\n");
}

internal void
test_log(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_log: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 46546786674687468ULL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    u32 J = env->iT / 3;

    u32 EIGHT[elemCount];
    xf_copy(elemCount, gXF_Four, EIGHT);
    xf_naive_mul2(elemCount, EIGHT);
    u32 SIXTEEN[elemCount];
    xf_copy(elemCount, EIGHT, SIXTEEN);
    xf_naive_mul2(elemCount, SIXTEEN);
    u32 FIVETEEN[elemCount];
    xf_sub(elemCount, SIXTEEN, gXF_One, FIVETEEN);

    u32 *TENTH = &gXF_Tenths[0][0];

    u32 log_17over16[elemCount];
    xf_add(elemCount, SIXTEEN, gXF_One, log_17over16);
    xf_div(elemCount, log_17over16, SIXTEEN, log_17over16);
    xf_log(elemCount, log_17over16, log_17over16);

    u32 log10_11over10[elemCount];
    xf_add(elemCount, &gXF_Tens[0][0], gXF_One, log10_11over10);
    xf_div(elemCount, log10_11over10, &gXF_Tens[0][0], log10_11over10);
    xf_log10(elemCount, log10_11over10, log10_11over10);

    u32 A[elemCount];
    u32 B[elemCount];
    u32 C[elemCount];
    xf_copy(elemCount, gXF_One, C);

    for (u32 i = 0; i < J; ++i)
    {
        xf_div(elemCount, C, BETA, C);
    }

    xf_add(elemCount, gXF_One, C, B);
    xf_sub(elemCount, gXF_One, C, A);

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    // NOTE(michiel): Random argument accuracy tests
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];
    u32 DEL[elemCount];
    u32 XL[elemCount];

    u32 X[elemCount];
    u32 Y[elemCount];
    u32 Z[elemCount];
    u32 ZZ[elemCount];
    u32 W[elemCount];

    for (u32 j = 0; j < 4; ++j)
    {
        u32 K1 = 0;
        u32 K3 = 0;
        xf_copy(elemCount, gXF_Zero, X1);
        xf_copy(elemCount, gXF_Zero, R6);
        xf_copy(elemCount, gXF_Zero, R7);
        xf_sub(elemCount, B, A, DEL);
        xf_div(elemCount, DEL, XN, DEL);
        xf_copy(elemCount, A, XL);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_unilateral(series, elemCount, X);
            xf_mul(elemCount, DEL, X, X);
            xf_add(elemCount, X, XL, X);

            if (j == 0)
            {
                xf_sub(elemCount, X, gXF_Half, Y);
                xf_sub(elemCount, Y, gXF_Half, Y);
                xf_log(elemCount, X, ZZ);
                xf_div(elemCount, gXF_One, gXF_Three, Z);
                xf_naive_div2(elemCount, Y);
                xf_naive_div2(elemCount, Y);
                xf_sub(elemCount, Z, Y, Z); // Z = Z - Y/4
                xf_naive_mul2(elemCount, Y);
                xf_naive_mul2(elemCount, Y);
                xf_mul(elemCount, Y, Z, Z);
                xf_sub(elemCount, Z, gXF_Half, Z);
                xf_mul(elemCount, Z, Y, Z);
                xf_mul(elemCount, Z, Y, Z);
                xf_add(elemCount, Z, Y, Z);
            }
            else if (j == 1)
            {
                xf_add(elemCount, X, EIGHT, X);
                xf_sub(elemCount, X, EIGHT, X);
                xf_div(elemCount, X, SIXTEEN, Y);
                xf_add(elemCount, X, Y, Y);
                xf_log(elemCount, X, Z);
                xf_log(elemCount, Y, ZZ);
                xf_sub(elemCount, ZZ, log_17over16, ZZ);
            }
            else if (j == 2)
            {
                xf_add(elemCount, X, EIGHT, X);
                xf_sub(elemCount, X, EIGHT, X);
                xf_mul(elemCount, X, TENTH, Y);
                xf_add(elemCount, X, Y, Y);
                xf_log10(elemCount, X, Z);
                xf_log10(elemCount, Y, ZZ);
                xf_sub(elemCount, ZZ, log10_11over10, ZZ);
            }
            else
            {
                xf_mul(elemCount, X, X, Z);
                xf_log(elemCount, Z, Z);
                xf_log(elemCount, X, ZZ);
                xf_naive_mul2(elemCount, ZZ);
            }

            xf_copy(elemCount, gXF_One, W);
            if (xf_compare(elemCount, Z, gXF_Zero)) // Z != 0
            {
                xf_sub(elemCount, Z, ZZ, W);
                xf_div(elemCount, W, ZZ, W);
            }

            u32 sign = xf_get_sign(elemCount, Z);
            xf_copy(elemCount, W, Z);
            if (sign)
            {
                xf_make_negative(elemCount, Z);
            }
            else
            {
                xf_make_positive(elemCount, Z);
            }

            s32 compare0 = xf_compare(elemCount, Z, gXF_Zero);
            if (compare0 > 0) // Z > 0
            {
                ++K1;
            }
            if (compare0 < 0) // Z < 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);
            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
            xf_add(elemCount, XL, DEL, XL);
        }

        u32 K2 = N - K3 - K1;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        if (j == 0)
        {
            fprintf(stdout, "\nTest of log(X) vs T.S. log(1+Y)\n");
        }
        else if (j == 1)
        {
            fprintf(stdout, "\nTest of log(X) vs log(17X/16) - log(17/16)\n");
        }
        else if (j == 2)
        {
            fprintf(stdout, "\nTest of log10(X) vs log10(11X/10) - log(11/10)\n");
        }
        else
        {
            fprintf(stdout, "\nTest of log(X*X) vs 2*log(X)\n");
        }

        if (j == 0)
        {
            fprintf(stdout, "%u random arguments were tested from the interval (1-eps, 1+eps), where eps = ", N);
            xf_print(elemCount, C, 4);
            fprintf(stdout, "\n");
        }
        else
        {
            fprintf(stdout, "%u random arguments were tested from the interval (", N);
            xf_print(elemCount, A, 4);
            fprintf(stdout, ", ");
            xf_print(elemCount, B, 4);
            fprintf(stdout, ")\n");
        }

        fprintf(stdout, "log%s(X) was larger %u times, agreed %u times and was smaller %u times\n", j == 2 ? "10" : "", K1, K2, K3);
        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        if (j == 0)
        {
            xf_square_root(elemCount, gXF_Half, A);
            xf_copy(elemCount, FIVETEEN, B);
            xf_div(elemCount, B, SIXTEEN, B);
        }
        else if (j == 1)
        {
            xf_square_root(elemCount, TENTH, A);
            xf_from_s32(elemCount, 9, B);
            xf_div(elemCount, B, &gXF_Tens[0][0], B);
        }
        else
        {
            xf_copy(elemCount, SIXTEEN, A);
            xf_from_s32(elemCount, 240, B);
        }
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nSpecial tests\n");
    fprintf(stdout, "The identity log(X) = -log(1/X) will be tested\n");

    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_add(elemCount, X, X, X);
        xf_add(elemCount, X, FIVETEEN, X);
        xf_div(elemCount, gXF_One, X, Y);
        xf_log(elemCount, Y, Y);
        xf_log(elemCount, X, Z);
        xf_add(elemCount, Z, Y, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\nTest of special arguments\n");
    xf_copy(elemCount, gXF_One, X);
    xf_log(elemCount, X, Y);
    fprintf(stdout, "log(1) = ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, "\n");
    xf_copy(elemCount, env->xMin, X);
    xf_log(elemCount, X, Y);
    fprintf(stdout, "log(xmin) = log(");
    xf_print(elemCount, X, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_copy(elemCount, env->xMax, X);
    xf_log(elemCount, X, Y);
    fprintf(stdout, "log(xmax) = log(");
    xf_print(elemCount, X, 4);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of errors\n");
    xf_copy(elemCount, gXF_Two, X);
    xf_make_negative(elemCount, X);
    fprintf(stdout, "log() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n");
    // This should trigger a error message
    xf_log(elemCount, X, Y);
    fprintf(stdout, "log returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_copy(elemCount, gXF_Zero, X);
    fprintf(stdout, "log() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n");
    xf_log(elemCount, X, Y);
    fprintf(stdout, "log returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of log\n\n");
}

// NOTE(michiel): |e^(-1/16) - 1|
global String gExpMin1Over16_MinusOne = static_string("0.060586937186524213880289175377694915475319109450558177990507337946459277757636680709074019829510656344025132618542987666048045211662048406935906860185518144097688882492696250521059998737269958821827694301578903098622721356314238025208823918975673927372077467983213592856048969682188200379114680534159481498988964263200439721182193028249874608994576863423217224624191904052888365682566319478808228007701487571728265254890170245385442233875620445113371480902944118914235229715441094015417611442612685883847987328767647685169609528610592493393096094352080441101894665373508761556228015526947248174991206677476357498065210517384055010910660416220422430285794329545698682830519352283898519966948312555957937097364871930264074467728970170186524292619659395205937898975474465200011308768655813859906790060250104391261740952374901873705788072492890410705564308058917860791942863630107678781670941335883623399019425202485580315550160023413338030350049526743013289394569106033088063947748632871404751839838522942148880658372277928127951508653880723177370052783105430233139976808210913017389911721533272619817839121848685625993499334872274133622276414381290256510922572913578676343869226488228293707765753544984772692711578472954978610351457149732903227258340698117895192204202169027404929050950051547017672889737196021845077615461554086234860308531172863993154421888473232134908801055329860550519682443869058000936221922567438504486650554079199857068053535647307589168835505275539943201045441665889904040832273588224597952503708100717256566347227164081805172537815399330785544407851923277676727527226117900128408091820435971363564582488563329184613852734534082185150363983404939611156547749989305558342755686109262001493629850854822761988538045177287182165799560938424115628605975345403898574264262067935773735615407789074208777004");

// NOTE(michiel): |e^(-45/16) - 0.0625|
global String gExpMin45Over16_Minus1Over16 = static_string("0.002445332104692057038944386692209648655994744180777192314986246920740125157726910220042152126554352510862427985320211711548706267777009970616661404315057600564255823737088926117257735881566621083580644414622909486014341650820732413738258701461840169333371155410367627055429098244550916631848125335961947460419137473959062091192822616616229783329108921466082521280271821562616460367334735741672499957660601223874383212234639221386358724917008381872600475288930733555589431416273529498293950230604700509408070083763043903122587896870117314882418811160239915845300979046349830229761613747236311261750215767506998271015507646063928707684844734229544047891392416882596026820424365605389422454556721871754581682925562087976707351464886953880670954634336088832387672196713518372171330739808385360255766466970623331424174876677640719519878401899873226062067539288505380140918992431217372623085300476782856329042437594853852471434777080230877704461174352674491943188351819616568106368905518495160232425294388816155136284430465236323905881065443273812308203113260106996917823579433569329640699977702304874200147077593621088592239906554094089475913203717678897748473821124417893877029683342264515526386136941970085686411949896066757813173028241701067360051542900112220087985404503522219246289761128208378456320614363452178364560778690384785919167298442701037459714286839342230850358143717561345778855981286892579152913362809497700563787455636694759567879081522122207516711107606747908855604899316325636487486514280494513307287040190733568457047245138390649767920127230768467841685160149686134047827181990517416949416868524091817854167877558548264898756268928416827882321941971264955758396392349877797033194354943344634168449693852035631164640381014236845239435530131634934804274037103745101447514214107254621431285202298386051768545");

internal void
test_exp(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_exp: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 182546891254ULL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    u32 expMod0[elemCount];
    u32 expMod1[elemCount];
    xf_from_string(elemCount, gExpMin1Over16_MinusOne, expMod0);
    xf_from_string(elemCount, gExpMin45Over16_Minus1Over16, expMod1);

    u32 oneOver16[elemCount];
    u32 V[elemCount];
    xf_copy(elemCount, gXF_One, V);
    xf_set_exponent(elemCount, V, XFLOAT_EXP_BIAS - 3); // NOTE(michiel): 1 / 16
    xf_copy(elemCount, V, oneOver16);

    u32 A[elemCount];
    u32 B[elemCount];
    xf_copy(elemCount, gXF_Log2, B);
    xf_naive_div2(elemCount, B);
    xf_sub(elemCount, V, B, A);

    u32 D[elemCount];
    xf_from_f32(elemCount, 0.9f, D);
    xf_mul(elemCount, D, env->xMax, D);
    xf_log(elemCount, D, D);

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    // NOTE(michiel): Random argument accuracy tests
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];
    u32 DEL[elemCount];
    u32 XL[elemCount];

    u32 X[elemCount];
    u32 Y[elemCount];
    u32 Z[elemCount];
    u32 ZZ[elemCount];
    u32 W[elemCount];

    for (u32 j = 0; j < 3; ++j)
    {
        u32 K1 = 0;
        u32 K3 = 0;
        xf_clear(elemCount, X1);
        xf_clear(elemCount, R6);
        xf_clear(elemCount, R7);
        xf_sub(elemCount, B, A, DEL);
        xf_div(elemCount, DEL, XN, DEL);
        xf_copy(elemCount, A, XL);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_unilateral(series, elemCount, X);
            xf_mul(elemCount, DEL, X, X);
            xf_add(elemCount, X, XL, X);

            xf_sub(elemCount, X, V, Y);
            if (xf_compare(elemCount, Y, gXF_Zero) < 0) // Y < 0
            {
                xf_add(elemCount, Y, V, X);
            }

            xf_exp(elemCount, X, Z);
            xf_exp(elemCount, Y, ZZ);

            if (j == 0)
            {
                xf_mul(elemCount, Z, expMod0, Y);
                xf_sub(elemCount, Z, Y, Z);
            }
            else
            {
                xf_mul(elemCount, Z, expMod1, Y);
                xf_mul(elemCount, Z, oneOver16, Z);
                xf_sub(elemCount, Z, Y, Z);
            }

            xf_copy(elemCount, gXF_One, W);
            if (xf_compare(elemCount, ZZ, gXF_Zero)) // ZZ != 0
            {
                xf_sub(elemCount, Z, ZZ, W);
                xf_div(elemCount, W, ZZ, W);
            }

            s32 compare0 = xf_compare(elemCount, W, gXF_Zero);
            if (compare0 < 0) // W < 0
            {
                ++K1;
            }
            if (compare0 > 0) // W > 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);
            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
            xf_add(elemCount, XL, DEL, XL);
        }

        u32 K2 = N - K3 - K1;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        fprintf(stdout, "\nTest of exp(X-");
        xf_print(elemCount, V, 4);
        fprintf(stdout, ") VS exp(X)/exp(");
        xf_print(elemCount, V, 4);
        fprintf(stdout, ")\n");
        fprintf(stdout, "%u random arguments were tested from the interval (", N);
        xf_print(elemCount, A, 4);
        fprintf(stdout, ", ");
        xf_print(elemCount, B, 4);
        fprintf(stdout, ")\n");

        fprintf(stdout, "exp(X-V) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        if (j == 0)
        {
            xf_from_s32(elemCount, 45, A);
            xf_mul(elemCount, V, A, V); // NOTE(michiel): 45 / 16
            xf_mul(elemCount, &gXF_Tens[0][0], B, A);
            xf_negate(elemCount, A);

            xf_mul(elemCount, gXF_Four, env->xMin, B);         // B = 4 * xmin
            u32 exponent = xf_get_exponent(elemCount, B);
            xf_set_exponent(elemCount, B, exponent + env->iT); // B *= 2^iT
            xf_log(elemCount, B, B);
        }
        else
        {
            xf_naive_mul2(elemCount, A);
            xf_negate(elemCount, A);
            xf_mul(elemCount, &gXF_Tens[0][0], A, B);
            if (xf_compare(elemCount, B, D) < 0) // B < D
            {
                xf_copy(elemCount, D, B);
            }
        }
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nSpecial tests\n");
    fprintf(stdout, "The identity exp(X)*exp(-X) = 1.0 will be tested\n"); //  (X, id-1.0)

    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_mul(elemCount, X, BETA, X);
        xf_copy(elemCount, X, Y);
        xf_negate(elemCount, Y);
        xf_exp(elemCount, X, Z);
        xf_exp(elemCount, Y, Y);
        xf_mul(elemCount, Z, Y, Z);
        xf_sub(elemCount, Z, gXF_One, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\nTest of special arguments\n");
    xf_clear(elemCount, X);
    xf_exp(elemCount, X, Y);
    xf_sub(elemCount, Y, gXF_One, Y);
    fprintf(stdout, "exp(0.0) - 1.0 = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_log(elemCount, env->xMin, X);
    xf_truncate(elemCount, X, X);
    xf_exp(elemCount, X, Y);
    fprintf(stdout, "exp(");
    xf_print(elemCount, X);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_log(elemCount, env->xMax, X);
    xf_truncate(elemCount, X, X);
    xf_exp(elemCount, X, Y);
    fprintf(stdout, "exp(");
    xf_print(elemCount, X);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_naive_div2(elemCount, X);
    xf_copy(elemCount, X, V);
    xf_naive_div2(elemCount, V);
    xf_exp(elemCount, X, Y);
    xf_exp(elemCount, V, Z);
    xf_mul(elemCount, Z, Z, Z);
    fprintf(stdout, "If exp(");
    xf_print(elemCount, X);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, " is not about\n   exp(");
    xf_print(elemCount, V);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Z);
    fprintf(stdout, " there is an argument reduction error\n");

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of errors\n");
    xf_square_root(elemCount, env->xMin, X);
    xf_div(elemCount, gXF_One, X, X);
    xf_negate(elemCount, X);
    fprintf(stdout, "exp() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n");
    xf_exp(elemCount, X, Y);
    fprintf(stdout, "exp returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_negate(elemCount, X);
    fprintf(stdout, "exp() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n");
    xf_exp(elemCount, X, Y);
    fprintf(stdout, "exp returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of exp\n\n");
}

internal void
test_pow(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_pow: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 9186256958162984ULL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    u32 ALXMAX[elemCount];
    xf_log(elemCount, env->xMax, ALXMAX);

    u32 ONEP5[elemCount];
    xf_div(elemCount, gXF_Three, gXF_Two, ONEP5);

    u32 SCALE[elemCount];
    xf_copy(elemCount, gXF_One, SCALE);

    for (u32 i = 0; i < (env->iT + 1) / 2; ++i)
    {
        xf_mul(elemCount, SCALE, BETA, SCALE);
    }

    u32 A[elemCount];
    u32 B[elemCount];
    u32 C[elemCount];

    xf_log(elemCount, env->xMin, C);
    xf_negate(elemCount, C);
    xf_maximum(elemCount, C, ALXMAX, C);
    xf_negate(elemCount, C);

    xf_from_f32(elemCount, 100.0f, A);
    xf_log(elemCount, A, A);

    xf_div(elemCount, C, A, C);      // C = -max(ALXMAX, -log(XMIN))/log(100.0)
    xf_copy(elemCount, gXF_Half, A);
    xf_copy(elemCount, gXF_One, B);

    u32 DELY[elemCount];
    xf_copy(elemCount, C, DELY);
    xf_negate(elemCount, DELY);
    xf_sub(elemCount, DELY, C, DELY); // DELY = -C - C

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    u32 Y1[elemCount];
    xf_clear(elemCount, Y1);

    // NOTE(michiel): Random argument accuracy tests
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];
    u32 DEL[elemCount];
    u32 XL[elemCount];

    u32 X[elemCount];
    u32 XSQ[elemCount];
    u32 Y[elemCount];
    u32 Y2[elemCount];
    u32 Z[elemCount];
    u32 ZZ[elemCount];
    u32 W[elemCount];

    for (u32 j = 0; j < 4; ++j)
    {
        u32 K1 = 0;
        u32 K3 = 0;
        xf_clear(elemCount, X1);
        xf_clear(elemCount, R6);
        xf_clear(elemCount, R7);
        xf_sub(elemCount, B, A, DEL);
        xf_div(elemCount, DEL, XN, DEL);
        xf_copy(elemCount, A, XL);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_unilateral(series, elemCount, X);
            xf_mul(elemCount, DEL, X, X);
            xf_add(elemCount, X, XL, X);

            if (j == 0)
            {
                xf_pow(elemCount, X, gXF_One, ZZ);
                xf_copy(elemCount, X, Z);
            }
            else
            {
                xf_mul(elemCount, SCALE, X, W);
                xf_add(elemCount, X, W, X);
                xf_sub(elemCount, X, W, X); // NOTE(michiel): Input sanitization (clear lowest bits if needed)
                xf_mul(elemCount, X, X, XSQ);

                if (j != 3)
                {
                    xf_pow(elemCount, XSQ, ONEP5, ZZ);
                    xf_mul(elemCount, X, XSQ, Z);
                }
                else
                {
                    xf_random_unilateral(series, elemCount, Y);
                    xf_mul(elemCount, DELY, Y, Y);
                    xf_add(elemCount, Y, C, Y);
                    xf_div(elemCount, Y, gXF_Two, Y2);
                    xf_add(elemCount, Y2, Y, Y2);
                    xf_sub(elemCount, Y2, Y, Y2); // NOTE(michiel): Input sanitization (clear lowest bits if needed)
                    xf_add(elemCount, Y2, Y2, Y);
                    xf_pow(elemCount, X, Y, Z);
                    xf_pow(elemCount, XSQ, Y2, ZZ);
                }
            }

            xf_copy(elemCount, gXF_One, W);
            if (xf_compare(elemCount, Z, gXF_Zero)) // Z != 0
            {
                xf_sub(elemCount, Z, ZZ, W);
                xf_div(elemCount, W, Z, W);
            }

            s32 compare0 = xf_compare(elemCount, W, gXF_Zero);
            if (compare0 > 0) // W > 0
            {
                ++K1;
            }
            if (compare0 < 0) // W < 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);

            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
                if (j == 3)
                {
                    xf_copy(elemCount, Y, Y1);
                }
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
            xf_add(elemCount, XL, DEL, XL);
        }

        u32 K2 = N - K3 - K1;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        if (j == 0)
        {
            fprintf(stdout, "\nTest of X^1.0 VS X\n");
            fprintf(stdout, "%u random arguments were tested from the interval (", N);
            xf_print(elemCount, A, 4);
            fprintf(stdout, ", ");
            xf_print(elemCount, B, 4);
            fprintf(stdout, ")\n");
            fprintf(stdout, "X^1.0 was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }
        else
        {
            if (j < 3)
            {
                fprintf(stdout, "\nTest of XSQ^1.5 VS XSQ*X\n");
                fprintf(stdout, "%u random arguments were tested from the interval (", N);
                xf_print(elemCount, A, 4);
                fprintf(stdout, ", ");
                xf_print(elemCount, B, 4);
                fprintf(stdout, ")\n");
                fprintf(stdout, "X^1.5 was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
            }
            else
            {
                fprintf(stdout, "\nTest of X^Y VS XSQ^(Y/2)");
                xf_add(elemCount, C, DELY, W);
                fprintf(stdout, "%u random arguments were tested from the region X in (", N);
                xf_print(elemCount, A, 4);
                fprintf(stdout, ", ");
                xf_print(elemCount, B, 4);
                fprintf(stdout, "), Y in (");
                xf_print(elemCount, C, 4);
                fprintf(stdout, ", ");
                xf_print(elemCount, W, 4);
                fprintf(stdout, ")\n");
                fprintf(stdout, "X^Y was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
            }
        }

        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        if (j != 4)
        {
            fprintf(stdout, "The maximum relative error of ");
            xf_print(elemCount, R6, 4);
            fprintf(stdout, " = 2^");
            xf_print(elemCount, W, 4);
            fprintf(stdout, " occured for X = ");
            xf_print(elemCount, X1, 4);
            fprintf(stdout, "\n");
        }
        else
        {
            fprintf(stdout, "The maximum relative error of ");
            xf_print(elemCount, R6, 4);
            fprintf(stdout, " = 2^");
            xf_print(elemCount, W, 4);
            fprintf(stdout, " occured for X = ");
            xf_print(elemCount, X1, 4);
            fprintf(stdout, " Y = ");
            xf_print(elemCount, Y1, 4);
            fprintf(stdout, "\n");
        }

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        if (j > 0)
        {
            if (j == 2)
            {
                xf_copy(elemCount, &gXF_Tens[0][0], B);
                xf_copy(elemCount, &gXF_Tenths[1][0], A);
            }
            else
            {
                xf_copy(elemCount, gXF_One, A);
                xf_div(elemCount, ALXMAX, gXF_Three, B);
                xf_exp(elemCount, B, B);
            }
        }
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nSpecial tests\n");
    fprintf(stdout, "The identity X^Y = (1/X)^(-Y) will be tested\n");

    xf_copy(elemCount, &gXF_Tens[0][0], B);

    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_mul(elemCount, X, B, X);
        xf_add(elemCount, X, gXF_One, X);
        xf_random_unilateral(series, elemCount, Y);
        xf_mul(elemCount, Y, B, Y);
        xf_add(elemCount, Y, gXF_One, Y);

        xf_pow(elemCount, X, Y, Z);
        xf_div(elemCount, gXF_One, X, ZZ);
        xf_negate(elemCount, Y);
        xf_pow(elemCount, ZZ, Y, ZZ);
        xf_negate(elemCount, Y);

        xf_sub(elemCount, Z, ZZ, W);
        xf_div(elemCount, W, Z, W);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Y, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, W);
        fprintf(stdout, "\n");
    }

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of errors\n");
    xf_copy(elemCount, BETA, X);
    xf_from_s32(elemCount, env->minExp, Y);
    xf_print(elemCount, X, 4);
    fprintf(stdout, " ^ ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, " will be computed.\n"); // No error
    xf_pow(elemCount, X, Y, Z);
    fprintf(stdout, "pow returned the value ");
    xf_print(elemCount, Z);
    fprintf(stdout, "\n");
    xf_from_s32(elemCount, env->maxExp - 1, Y);
    xf_print(elemCount, X, 4);
    fprintf(stdout, " ^ ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, " will be computed.\n"); // No error
    xf_pow(elemCount, X, Y, Z);
    fprintf(stdout, "pow returned the value ");
    xf_print(elemCount, Z);
    fprintf(stdout, "\n");
    xf_clear(elemCount, X);
    xf_copy(elemCount, gXF_Two, Y);
    xf_print(elemCount, X, 4);
    fprintf(stdout, " ^ ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, " will be computed.\n"); // No error
    xf_pow(elemCount, X, Y, Z);
    fprintf(stdout, "pow returned the value ");
    xf_print(elemCount, Z);
    fprintf(stdout, "\n");
    xf_copy(elemCount, Y, X);
    xf_negate(elemCount, X);
    xf_clear(elemCount, Y);
    xf_print(elemCount, X, 4);
    fprintf(stdout, " ^ ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, " will be computed.\n"); // Error
    xf_pow(elemCount, X, Y, Z);
    fprintf(stdout, "pow returned the value ");
    xf_print(elemCount, Z);
    fprintf(stdout, "\n");
    xf_copy(elemCount, gXF_Two, Y);
    xf_print(elemCount, X, 4);
    fprintf(stdout, " ^ ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, " will be computed.\n"); // Error
    xf_pow(elemCount, X, Y, Z);
    fprintf(stdout, "pow returned the value ");
    xf_print(elemCount, Z);
    fprintf(stdout, "\n");
    xf_clear(elemCount, X);
    xf_clear(elemCount, Y);
    xf_print(elemCount, X, 4);
    fprintf(stdout, " ^ ");
    xf_print(elemCount, Y, 4);
    fprintf(stdout, " will be computed.\n"); // Error
    xf_pow(elemCount, X, Y, Z);
    fprintf(stdout, "pow returned the value ");
    xf_print(elemCount, Z);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of pow\n\n");
}

internal void
test_sincos(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_sincos: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 182546891254ULL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    u32 A[elemCount];
    u32 B[elemCount];
    u32 C[elemCount];
    xf_clear(elemCount, A);
    xf_copy(elemCount, gXF_PiOver2, B);
    xf_copy(elemCount, gXF_PiOver2, C);

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    // NOTE(michiel): Random argument accuracy tests
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];
    u32 DEL[elemCount];
    u32 XL[elemCount];

    u32 X[elemCount];
    u32 Y[elemCount];
    u32 Z[elemCount];
    u32 ZZ[elemCount];
    u32 W[elemCount];

    for (u32 j = 0; j < 3; ++j)
    {
        u32 K1 = 0;
        u32 K3 = 0;
        xf_clear(elemCount, X1);
        xf_clear(elemCount, R6);
        xf_clear(elemCount, R7);
        xf_sub(elemCount, B, A, DEL);
        xf_div(elemCount, DEL, XN, DEL);
        xf_copy(elemCount, A, XL);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_unilateral(series, elemCount, X);
            xf_mul(elemCount, DEL, X, X);
            xf_add(elemCount, X, XL, X);

            // NOTE(michiel): Input sanitization
            xf_div(elemCount, X, gXF_Three, Y);
            xf_add(elemCount, X, Y, Y);
            xf_sub(elemCount, Y, X, Y);
            xf_mul(elemCount, Y, gXF_Three, X);

            if (j < 2)
            {
                xf_sin(elemCount, X, Z);
                xf_sin(elemCount, Y, ZZ);
                if (xf_compare(elemCount, Z, gXF_Zero) != 0)
                {
                    xf_mul(elemCount, ZZ, ZZ, W);
                    xf_mul(elemCount, gXF_Four, W, W);
                    xf_sub(elemCount, gXF_Three, W, W);
                    xf_mul(elemCount, ZZ, W, W);
                    xf_sub(elemCount, Z, W, W);
                    xf_div(elemCount, W, Z, W);
                }
                else
                {
                    xf_copy(elemCount, gXF_One, W);
                }
            }
            else
            {
                xf_cos(elemCount, X, Z);
                xf_cos(elemCount, Y, ZZ);
                if (xf_compare(elemCount, Z, gXF_Zero) != 0)
                {
                    xf_mul(elemCount, ZZ, ZZ, W);
                    xf_mul(elemCount, gXF_Four, W, W);
                    xf_sub(elemCount, gXF_Three, W, W);
                    xf_mul(elemCount, ZZ, W, W);
                    xf_add(elemCount, Z, W, W);
                    xf_div(elemCount, W, Z, W);
                }
                else
                {
                    xf_copy(elemCount, gXF_One, W);
                }
            }

            s32 compare0 = xf_compare(elemCount, W, gXF_Zero);
            if (compare0 > 0) // W > 0
            {
                ++K1;
            }
            if (compare0 < 0) // W < 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);

            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
            xf_add(elemCount, XL, DEL, XL);
        }

        u32 K2 = N - K3 - K1;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        if (j < 2)
        {
            fprintf(stdout, "\nTest of sin(X) VS 3*sin(X/3)-4*sin(X/3)^3\n");
            fprintf(stdout, "%u random arguments were tested from the interval (", N);
            xf_print(elemCount, A, 4);
            fprintf(stdout, ", ");
            xf_print(elemCount, B, 4);
            fprintf(stdout, ")\n");
            fprintf(stdout, "sin(X) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }
        else
        {
            fprintf(stdout, "\nTest of cos(X) VS 4*cos(X/3)^3-3*cos(X/3)\n");
            fprintf(stdout, "%u random arguments were tested from the interval (", N);
            xf_print(elemCount, A, 4);
            fprintf(stdout, ", ");
            xf_print(elemCount, B, 4);
            fprintf(stdout, ")\n");
            fprintf(stdout, "cos(X) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }

        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        if (j == 1)
        {
            xf_add(elemCount, B, C, A);
        }
        else
        {
            xf_mul(elemCount, gXF_Six, gXF_Pi, A);
        }
        xf_add(elemCount, A, C, B);
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nSpecial tests\n");

    u32 BETAP[elemCount];
    xf_copy(elemCount, gXF_One, BETAP);
    xf_set_exponent(elemCount, BETAP, XFLOAT_EXP_BIAS + (env->iT / 2));
    xf_div(elemCount, gXF_One, BETAP, C);
    xf_add(elemCount, A, C, X);
    xf_sin(elemCount, X, Z);
    xf_sub(elemCount, A, C, X);
    xf_sin(elemCount, X, X);
    xf_sub(elemCount, Z, X, Z);
    xf_add(elemCount, C, C, X);
    xf_div(elemCount, Z, X, Z);
    fprintf(stdout, "If ");
    xf_print(elemCount, Z, 20);
    fprintf(stdout, " is not almost 1.0, sin has the wrong period.\n");

    fprintf(stdout, "\nThe identity sin(-X) = -sin(X) will be tested\n"); // sin(-X) + sin(X) == 0

    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_mul(elemCount, X, A, X);
        xf_sin(elemCount, X, Y);
        xf_negate(elemCount, X);
        xf_sin(elemCount, X, Z);
        xf_add(elemCount, Z, Y, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\nThe identity sin(X) = X, for small X, will be tested\n");
    xf_set_exponent(elemCount, BETAP, XFLOAT_EXP_BIAS + env->iT);
    xf_random_unilateral(series, elemCount, X);
    xf_div(elemCount, X, BETAP, X);

    for (u32 i = 0; i < 5; ++i)
    {
        xf_sin(elemCount, X, Z);
        xf_sub(elemCount, X, Z, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
        xf_div(elemCount, X, gXF_Two, X);
    }

    fprintf(stdout, "\nThe identity cos(-X) = cos(X) will be tested\n");
    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_mul(elemCount, X, A, X);
        xf_cos(elemCount, X, Y);
        xf_negate(elemCount, X);
        xf_cos(elemCount, X, Z);
        xf_sub(elemCount, Z, Y, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\nTest of underflow for very small argument\n");
    u32 EXPON[elemCount];
    xf_from_s32(elemCount, env->minExp, EXPON);
    xf_from_f32(elemCount, 0.75f, X);
    xf_mul(elemCount, X, EXPON, EXPON);
    xf_pow(elemCount, gXF_Two, EXPON, X);
    xf_sin(elemCount, X, Y);
    fprintf(stdout, "sin(");
    xf_print(elemCount, X, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    fprintf(stdout, "\nThe following three lines illustrate the loss in significance for large arguments. The arguments are consecutive.\n");
    xf_square_root(elemCount, BETAP, Z);
    xf_sub(elemCount, gXF_One, env->epsNeg, X);
    xf_mul(elemCount, X, Z, X);
    xf_sin(elemCount, X, Y);
    fprintf(stdout, "sin(");
    xf_print(elemCount, X, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_sin(elemCount, Z, Y);
    fprintf(stdout, "sin(");
    xf_print(elemCount, Z, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_add(elemCount, gXF_One, env->eps, X);
    xf_mul(elemCount, X, Z, X);
    xf_sin(elemCount, X, Y);
    fprintf(stdout, "sin(");
    xf_print(elemCount, X, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of errors\n");

    xf_copy(elemCount, BETAP, X);
    fprintf(stdout, "sin() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n"); // error
    xf_sin(elemCount, X, Y);
    fprintf(stdout, "sin returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of sin/cos\n\n");
}

global String gTan11Fraction = static_string("0.9508464541951420257954832034531539516575098851311946310945881123661134104031654779989090875181672217233308822265082444677196937143193893426065148170417169441360752810352819635165893207630835309928699271953564327191966650623295663931499462491032676940456029120591388467964276901349435046331686140473021208667557700088944601073338960611827896379185288648924496562849281424574051006181018365689216220850954935450628914569262072859566649852728224365514201600888424246180522379527040303562592501755293425549205811756632002073239601692021469745769115251021322526574982191452687125405094639552969762003711001407400895767183275749190156893686935406007954770336613005330021472674834397142120385652261415713901224992215958650572846727110042457281785624822533459947082363810620609474626728556970630376157078590618628438046436611289060068327529424792559961054368938015539874139567289508604663993328637017805164672360545757520312616985257518067459231068740291033264624803396110227163746301891564542908956044661365347292452899842238269280292330337549334320277554154276021047469247255103385746387385197352225082901808074125723842175298558016470675037220064839467983723688870356566876210812840695318728873803474854648685626234022418474591517401173316511119320341043020716793644013078201675032368547309397813256895510442114346573658387766478031139033200631703089368522000973194949760970506120819017015064484610316046050213287267905217592664567121875029698804760777834296475555345921650224442572926122564679917783299646401607924167768553311592278066995513346767965081177969650841400883057572855455493922835636978554084570665981098504324172730829546385195222698314201827996592013007257367949979429935980528140471292257607809581729125557023129381730337718815632159701329525488266514862740810884445154125471240266511839009130296753042100067");

internal void
test_tancot(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_tancot: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 182546891254ULL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    u32 A[elemCount];
    u32 B[elemCount];
    xf_clear(elemCount, A);
    xf_copy(elemCount, gXF_PiOver2, B);
    xf_naive_div2(elemCount, B);            // PI / 4

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    // NOTE(michiel): Random argument accuracy tests
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];
    u32 DEL[elemCount];
    u32 XL[elemCount];

    u32 X[elemCount];
    u32 Y[elemCount];
    u32 Z[elemCount];
    u32 ZZ[elemCount];
    u32 W[elemCount];

    for (u32 j = 0; j < 4; ++j)
    {
        u32 K1 = 0;
        u32 K3 = 0;
        xf_clear(elemCount, X1);
        xf_clear(elemCount, R6);
        xf_clear(elemCount, R7);
        xf_sub(elemCount, B, A, DEL);
        xf_div(elemCount, DEL, XN, DEL);
        xf_copy(elemCount, A, XL);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_unilateral(series, elemCount, X);
            xf_mul(elemCount, DEL, X, X);
            xf_add(elemCount, X, XL, X);

            xf_mul(elemCount, X, gXF_Half, Y);
            xf_add(elemCount, Y, Y, X);

            if (j < 3)
            {
                xf_tan(elemCount, X, Z);
                xf_tan(elemCount, Y, ZZ);
                if (xf_compare(elemCount, Z, gXF_Zero))   // Z != 0         | TAN(X) != 0
                {
                    xf_sub(elemCount, gXF_Half, ZZ, Y);   // Y = 0.5 - ZZ
                    xf_add(elemCount, gXF_Half, Y, Y);    // Y += 0.5       | (0.5-ZZ)+0.5
                    xf_add(elemCount, gXF_Half, ZZ, W);   // W = 0.5 + ZZ
                    xf_add(elemCount, gXF_Half, W, W);    // W += 0.5       | (0.5+ZZ)+0.5
                    xf_mul(elemCount, Y, W, W);           // W *= Y         | ((0.5-ZZ)+0.5) + ((0.5+ZZ)+0.5)

                    xf_add(elemCount, ZZ, ZZ, Y);         // Y = ZZ + ZZ
                    xf_div(elemCount, Y, W, W);           // W = Y / W      | (ZZ+ZZ)/W
                    xf_sub(elemCount, Z, W, W);           // W = Z - W      | Z - (ZZ+ZZ)/W
                    xf_div(elemCount, W, Z, W);           // W /= Z         | (Z - (ZZ+ZZ)/W) / Z
                }
                else
                {
                    xf_copy(elemCount, gXF_One, W);
                }
            }
            else
            {
                xf_cot(elemCount, X, Z);
                xf_cot(elemCount, Y, ZZ);

                if (xf_compare(elemCount, Z, gXF_Zero))   // Z != 0         | COTAN(X) != 0
                {
                    xf_sub(elemCount, gXF_Half, ZZ, Y);   // Y = 0.5 - ZZ
                    xf_add(elemCount, gXF_Half, Y, Y);    // Y += 0.5       | (0.5-ZZ)+0.5
                    xf_add(elemCount, gXF_Half, ZZ, W);   // W = 0.5 + ZZ
                    xf_add(elemCount, gXF_Half, W, W);    // W += 0.5       | (0.5+ZZ)+0.5
                    xf_mul(elemCount, Y, W, W);           // W *= Y         | ((0.5-ZZ)+0.5) + ((0.5+ZZ)+0.5)

                    xf_add(elemCount, ZZ, ZZ, Y);         // Y = ZZ + ZZ
                    xf_div(elemCount, W, Y, W);           // W /= Y         | W/(ZZ+ZZ)
                    xf_add(elemCount, Z, W, W);           // W += Z         | Z + W/(ZZ+ZZ)
                    xf_div(elemCount, W, Z, W);           // W /= Z         | (Z + W/(ZZ+ZZ)) / Z
                }
                else
                {
                    xf_copy(elemCount, gXF_One, W);
                }
            }

            s32 compare0 = xf_compare(elemCount, W, gXF_Zero);
            if (compare0 > 0) // W > 0
            {
                ++K1;
            }
            if (compare0 < 0) // W < 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);

            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
            xf_add(elemCount, XL, DEL, XL);
        }

        u32 K2 = N - K3 - K1;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        if (j < 3)
        {
            fprintf(stdout, "\nTest of tan(X) VS 2*tan(X/2)/(1-tan(X/2)^2)\n");
        }
        else
        {
            fprintf(stdout, "\nTest of cot(X) VS (cot(X/2)^2-1)/(2*cot(X/2))\n");
        }

        fprintf(stdout, "%u random arguments were tested from the interval (", N);
        xf_print(elemCount, A, 4);
        fprintf(stdout, ", ");
        xf_print(elemCount, B, 4);
        fprintf(stdout, ")\n");

        if (j < 3)
        {
            fprintf(stdout, "tan(X) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }
        else
        {
            fprintf(stdout, "cot(X) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }

        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        if (j == 0)
        {
            xf_from_f32(elemCount, 0.875f, A);
            xf_mul(elemCount, A, gXF_Pi, A);
            xf_from_f32(elemCount, 1.125f, B);
            xf_mul(elemCount, B, gXF_Pi, B);
        }
        else
        {
            xf_mul(elemCount, gXF_Six, gXF_Pi, A);
            xf_copy(elemCount, gXF_PiOver2, B);
            xf_naive_div2(elemCount, B);
            xf_add(elemCount, A, B, B);
        }
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nSpecial tests\n");

    fprintf(stdout, "The identity tan(-X) = -tan(X) will be tested\n");
    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_mul(elemCount, X, A, X);
        xf_tan(elemCount, X, Z);
        xf_negate(elemCount, X);
        xf_tan(elemCount, X, Y);
        xf_negate(elemCount, X);
        xf_add(elemCount, Z, Y, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\nThe identity tan(X) = X, for small X, will be tested\n");
    u32 BETAP[elemCount];
    xf_copy(elemCount, gXF_One, BETAP);
    xf_set_exponent(elemCount, BETAP, XFLOAT_EXP_BIAS + env->iT);
    xf_random_unilateral(series, elemCount, X);
    xf_div(elemCount, X, BETAP, X);
    for (u32 i = 0; i < 5; ++i)
    {
        xf_tan(elemCount, X, Z);
        xf_sub(elemCount, X, Z, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
        xf_div(elemCount, X, gXF_Two, X);
    }

    fprintf(stdout, "\nTest of underflow for very small arguments\n");
    u32 EXPON[elemCount];
    xf_from_s32(elemCount, env->minExp, EXPON);
    xf_from_f32(elemCount, 0.75f, X);
    xf_mul(elemCount, X, EXPON, EXPON);
    xf_pow(elemCount, gXF_Two, EXPON, X);
    xf_tan(elemCount, X, Y);
    fprintf(stdout, "tan(");
    xf_print(elemCount, X, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    u32 tan11_C0[elemCount];
    u32 tan11_C1[elemCount];
    xf_from_s32(elemCount, 225, tan11_C0);
    xf_negate(elemCount, tan11_C0);
    xf_from_string(elemCount, gTan11Fraction, tan11_C1);
    xf_negate(elemCount, tan11_C1);
    xf_from_s32(elemCount, 11, X);
    xf_tan(elemCount, X, Y);
    xf_sub(elemCount, tan11_C0, Y, W);
    xf_add(elemCount, W, tan11_C1, W);
    xf_add(elemCount, tan11_C0, tan11_C1, A);
    xf_div(elemCount, W, A, W);
    xf_copy(elemCount, W, Z);
    xf_absolute(elemCount, Z);
    xf_log(elemCount, Z, Z);
    xf_div(elemCount, Z, ALBETA, Z);
    fprintf(stdout, "\nThe relative error in tan(11) is ");
    xf_print(elemCount, W, 7);
    fprintf(stdout, " = 2^");
    xf_print(elemCount, Z, 4);
    fprintf(stdout, "\ntan(");
    xf_print(elemCount, X, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    xf_add(elemCount, AIT, Z, W);
    xf_maximum(elemCount, W, gXF_Zero, W);
    fprintf(stdout, "The estimated loss of base 2 significant digits is "); // 1022
    xf_print(elemCount, W, 4);
    fprintf(stdout, "\n");

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of errors\n"); // 1050
    xf_naive_div2(elemCount, AIT);
    xf_pow(elemCount, gXF_Two, AIT, X);
    fprintf(stdout, "tan() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n"); // no error
    xf_tan(elemCount, X, Y);
    fprintf(stdout, "tan returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");
    xf_copy(elemCount, BETAP, X);
    fprintf(stdout, "tan() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n"); // error
    xf_tan(elemCount, X, Y);
    fprintf(stdout, "tan returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of tan/cot\n\n"); // 1100
}

global String gASinCosHalfPi = static_string("0.00048382679489661923132169163975144209858469968755291048747229615390820314310449931401741267105853399107404325664115332354692230477529111586267970406424055872514205135096926055277982231147447746519098221440548783296672306423782411689339158263560095457282428346173017430522716332410669680363012457063686229350330315779408744076046048141462704585768218394629518000566526527441023326069207347597075580471652863518287979597654609305869096630589655255927403723118998137478367594287636244561396909150597456491683668122032832154301069747319761236859535108993047185138526960858814658837619233740923383470256600028406357263178041389288567137889480458681858936073422045061247671507327479268552539613984462946177100997805606451098043201720907990681488738565498025935360567499999918648902497552986586640804815929751222972767345415132126115412667234251763096559408550500156891937644329376660419071030858883457365179912674521437773436557978143194117689379687597889092889026608561340330650096393830559795460821009946904762860053274293163943296807669091398411515097601765092648449788681129970694562486088764173956575778742862122707534797541476655843086392794453754919087731873246965962753020046385083556950492441200642918080178185383005235509097147779809947338391872472412768988736342355202376732310402334212953474564665683851449457605237608102848301202901907509675562669121501779382012374823663195709963630213496139839117739081800467086082060996229315751514309148727785337491925274729429346349784546360539875465147766058267249360137798011824033274955994091739887678318490371327126393127590920878733644548888639690004082353000807262459608660860738617507072098678427408068057867627606673787092473421926166195369707166727388120843125949178474278104960961109213627512712844383589524730082673340249431361639589304289219191398398834072705047694189318047534003211256260255869649244804206424431347280212098264251110533059315337213931101959747252356185689348047818218595864373388232878698120694543291632299790669523901379504973288203947563473419917629785491291131026124470386335973913424130073849545132006819721872765253410174812622587469982571571490459532962546861084823075785492919370529894297988648774946508087696423406913434193447138707799592796262297697971552498626234042299363682234792432691836811131304956230402562194219522562206827488139039885784571799885006480804472084743427792420317671103611291424432407922801425300842136972613373383944762606926127497733336391199322829805817744311528872824901779681728408716205625753803473972554829804701261443985544657283456843361437447028005075165430896434046043738045891246929450485745483799263068277489094656489241084149947436132940242878200713523877756618982072576187311718227142922239763293391052557067736786976155671358305106798476811572147624246859355507288270179513996720187100365528926953109919372390423924484166072285693437597175321510922659552424050268530734033745963909559896997603070983171437722032187256185909608999919550795978090733757134561987447045359324711598078397260404757327511261580194096507104688106892797831946889354151953489603867336109128129983075071075153401922386727460130270733296260074872142536625933300106621704409535524316586732482572695289813428050275405332939849908178736819202628572955144853207005548560314021951987975783857885021016893496800361527938158817971093656257356026646409591309306293366078959920742441458223530478763534786104587835583614554908454576400867533563742916114359176046769828625605417895756849410457221050337551673355515706335556849543292581991575098508257558425857188288091757782544245499492999119367276416581775382395926794661309274481606646654492853210233762953545774");

internal void
test_asincos(Environment *env, u32 elemCount)
{
    u64 timeRand = time(NULL);
    fprintf(stdout, "test_asincos: time rand %lu\n", timeRand);
    RandomSeriesPCG series_ = random_seed_pcg(timeRand, 182546891254ULL);
    RandomSeriesPCG *series = &series_;

    u32 BETA[elemCount];
    xf_copy(elemCount, gXF_Two, BETA);

    u32 ALBETA[elemCount];
    xf_copy(elemCount, gXF_Log2, ALBETA);

    u32 AIT[elemCount];
    xf_from_s32(elemCount, env->iT, AIT);

    s64 K;
    u32 temp[elemCount];
    xf_pow(elemCount, gXF_Two, AIT, temp);
    xf_log10(elemCount, temp, temp);
    K = xf_integer_fraction(elemCount, temp, temp) + 1;

    u32 C1[elemCount];
    u32 C2[elemCount];
    xf_from_s32(elemCount, 201, C1);
    xf_from_s32(elemCount, 128, C2);
    xf_div(elemCount, C1, C2, C1);
    xf_from_string(elemCount, gASinCosHalfPi, C2);
    //xf_copy(elemCount, gXF_PiOver2, C1);
    //xf_copy(elemCount, gXF_Zero, C2);

    u32 A[elemCount];
    u32 B[elemCount];
    xf_from_f32(elemCount, -0.125f, A);
    xf_from_f32(elemCount,  0.125f, B);

    u32 N = 2000;
    u32 XN[elemCount];
    xf_from_s32(elemCount, N, XN);

    s32 L = -1;

    // NOTE(michiel): Random argument accuracy tests
    u32 X1[elemCount];
    u32 R6[elemCount];
    u32 R7[elemCount];
    u32 DEL[elemCount];
    u32 XL[elemCount];

    u32 X[elemCount];
    u32 Y[elemCount];
    u32 YSQ[elemCount];
    u32 Z[elemCount];
    u32 ZZ[elemCount];
    u32 W[elemCount];

    u32 SUM[elemCount];
    u32 XM[elemCount];
    u32 S[elemCount];

    for (u32 j = 0; j < 5; ++j)
    {
        u32 K1 = 0;
        u32 K3 = 0;

        L = -L;

        xf_clear(elemCount, X1);
        xf_clear(elemCount, R6);
        xf_clear(elemCount, R7);
        xf_sub(elemCount, B, A, DEL);
        xf_div(elemCount, DEL, XN, DEL);
        xf_copy(elemCount, A, XL);

        for (u32 i = 0; i < N; ++i)
        {
            xf_random_unilateral(series, elemCount, X);
            xf_mul(elemCount, DEL, X, X);
            xf_add(elemCount, X, XL, X);

            if (j <= 1)
            {
                xf_copy(elemCount, X, Y);
                xf_mul(elemCount, Y, Y, YSQ);
            }
            else
            {
                xf_mul(elemCount, gXF_Half, X, YSQ);
                xf_absolute(elemCount, YSQ);
                xf_sub(elemCount, gXF_Half, YSQ, YSQ);  // YSQ = 0.5 - 0.5*ABS(X)
                xf_add(elemCount, YSQ, YSQ, X);
                xf_sub(elemCount, gXF_Half, X, X);
                xf_add(elemCount, X, gXF_Half, X);

                if (j == 4)
                {
                    xf_negate(elemCount, X);
                }

                xf_square_root(elemCount, YSQ, Y);
                xf_add(elemCount, Y, Y, Y);
            }

            xf_clear(elemCount, SUM);
            xf_from_s64(elemCount, K + K + 1, XM);

            if (L > 0)
            {
                xf_asin(elemCount, X, Z);
            }
            else
            {
                xf_acos(elemCount, X, Z);
            }

            for (s64 m = 0; m < K; ++m)
            {
                xf_div(elemCount, gXF_One, XM, temp);
                xf_add(elemCount, SUM, temp, SUM);
                xf_mul(elemCount, SUM, YSQ, SUM);
                xf_sub(elemCount, XM, gXF_Two, XM);
                xf_add(elemCount, XM, gXF_One, temp);
                xf_div(elemCount, XM, temp, temp);
                xf_mul(elemCount, SUM, temp, SUM);
            }

            xf_mul(elemCount, Y, SUM, SUM);

            if ((j != 0) && (j != 3))
            {
                xf_add(elemCount, C1, C2, S);
                xf_sub(elemCount, C1, S, temp);
                xf_add(elemCount, temp, C2, temp);
                xf_sub(elemCount, temp, SUM, SUM);
                xf_add(elemCount, S, SUM, ZZ);
                xf_sub(elemCount, S, ZZ, temp);
                xf_add(elemCount, temp, SUM, SUM);
                xf_sub(elemCount, SUM, Y, SUM);
                xf_copy(elemCount, ZZ, S);
                xf_add(elemCount, S, SUM, ZZ);
                xf_sub(elemCount, S, ZZ, temp);
                xf_add(elemCount, temp, SUM, SUM);
                if (!env->round)
                {
                    xf_add(elemCount, SUM, SUM, temp);
                    xf_add(elemCount, ZZ, temp, ZZ);
                }
            }
            else
            {
                xf_add(elemCount, Y, SUM, ZZ);
                xf_sub(elemCount, Y, ZZ, temp);
                xf_add(elemCount, temp, SUM, SUM);
                if (!env->round)
                {
                    xf_add(elemCount, SUM, SUM, temp);
                    xf_add(elemCount, ZZ, temp, ZZ);
                }
            }

            if (xf_compare(elemCount, Z, gXF_Zero) != 0) // Z != 0
            {
                xf_sub(elemCount, Z, ZZ, W);
                xf_div(elemCount, W, Z, W);  // W = (Z - ZZ) / Z
            }
            else
            {
                xf_copy(elemCount, gXF_One, W);
            }

            s32 compare0 = xf_compare(elemCount, W, gXF_Zero);
            if (compare0 > 0) // W > 0
            {
                ++K1;
            }
            if (compare0 < 0) // W < 0
            {
                ++K3;
            }

            xf_absolute(elemCount, W);

            if (xf_compare(elemCount, R6, W) < 0) // R6 < W
            {
                xf_copy(elemCount, W, R6);
                xf_copy(elemCount, X, X1);
            }
            xf_mul(elemCount, W, W, W);
            xf_add(elemCount, R7, W, R7);
            xf_add(elemCount, XL, DEL, XL);
        }

        u32 K2 = N - K3 - K1;
        xf_div(elemCount, R7, XN, R7);
        xf_square_root(elemCount, R7, R7);

        if (L > 0)
        {
            fprintf(stdout, "\nTest of asin(X) VS taylor series\n");
            fprintf(stdout, "%u random arguments were tested from the interval (", N);
            xf_print(elemCount, A, 4);
            fprintf(stdout, ", ");
            xf_print(elemCount, B, 4);
            fprintf(stdout, ")\n");
            fprintf(stdout, "asin(X) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }
        else
        {
            fprintf(stdout, "\nTest of acos(X) VS taylor series\n");
            fprintf(stdout, "%u random arguments were tested from the interval (", N);
            xf_print(elemCount, A, 4);
            fprintf(stdout, ", ");
            xf_print(elemCount, B, 4);
            fprintf(stdout, ")\n");
            fprintf(stdout, "acos(X) was larger %u times, agreed %u times and was smaller %u times\n", K1, K2, K3);
        }

        fprintf(stdout, "There are %u base 2 significant digits in a floating-point number.\n", env->iT);

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R6, gXF_Zero)) // R6 != 0
        {
            xf_copy(elemCount, R6, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_from_s32(elemCount, -999999, W);
        if (xf_compare(elemCount, R7, gXF_Zero)) // R7 != 0
        {
            xf_copy(elemCount, R7, W);
            xf_absolute(elemCount, W);
            xf_log(elemCount, W, W);
            xf_div(elemCount, W, ALBETA, W);
        }
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        xf_add(elemCount, AIT, W, W);
        xf_maximum(elemCount, W, gXF_Zero, W);
        fprintf(stdout, "The estimated loss of base 2 significant digits is ");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        if (j == 1)
        {
            xf_from_f32(elemCount, 0.75f, A);
            xf_copy(elemCount, gXF_One, B);
        }
        else if (j == 3)
        {
            xf_copy(elemCount, A, B);
            xf_negate(elemCount, B);
            xf_copy(elemCount, gXF_One, A);
            xf_negate(elemCount, A);
            xf_add(elemCount, C1, C1, C1);
            xf_add(elemCount, C2, C2, C2);
            L = -L;
        }
    }

    // NOTE(michiel): Special tests
    fprintf(stdout, "\nSpecial tests\n");

    fprintf(stdout, "The identity asin(-X) = -asin(X) will be tested\n");
    for (u32 i = 0; i < 5; ++i)
    {
        xf_random_unilateral(series, elemCount, X);
        xf_mul(elemCount, A, X, X);
        xf_negate(elemCount, X);
        xf_asin(elemCount, X, Y);
        xf_negate(elemCount, X);
        xf_asin(elemCount, X, Z);
        xf_add(elemCount, Z, Y, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
    }

    fprintf(stdout, "\nThe identity asin(X) = X, for small X, will be tested\n");
    u32 BETAP[elemCount];
    xf_copy(elemCount, gXF_One, BETAP);
    xf_set_exponent(elemCount, BETAP, XFLOAT_EXP_BIAS + env->iT + 1);
    xf_random_unilateral(series, elemCount, X);
    xf_div(elemCount, X, BETAP, X);
    for (u32 i = 0; i < 5; ++i)
    {
        xf_asin(elemCount, X, Z);
        xf_sub(elemCount, X, Z, Z);
        xf_print(elemCount, X, 8);
        fprintf(stdout, ", ");
        xf_print(elemCount, Z);
        fprintf(stdout, "\n");
        xf_div(elemCount, X, gXF_Two, X);
    }

    fprintf(stdout, "\nTest of underflow for very small argument\n");
    u32 EXPON[elemCount];
    xf_from_s32(elemCount, env->minExp, EXPON);
    xf_from_f32(elemCount, 0.75f, X);
    xf_mul(elemCount, X, EXPON, EXPON);
    xf_pow(elemCount, gXF_Two, EXPON, X);
    xf_asin(elemCount, X, Y);
    fprintf(stdout, "asin(");
    xf_print(elemCount, X, 6);
    fprintf(stdout, ") = ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    // NOTE(michiel): Error tests
    fprintf(stdout, "\nTest of errors\n");
    xf_from_f32(elemCount, 1.2f, X);
    fprintf(stdout, "sin() will be called with the argument ");
    xf_print(elemCount, X, 4);
    fprintf(stdout, "\n"); // error
    xf_asin(elemCount, X, Y);
    fprintf(stdout, "asin returned the value ");
    xf_print(elemCount, Y);
    fprintf(stdout, "\n");

    fprintf(stdout, "\nThis concludes the tests of asin/acos\n\n"); // 1100
}
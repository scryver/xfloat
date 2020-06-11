struct Environment
{
    // NOTE(michiel): iBeta is presumed to be 2
    u32 iT;

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

    fprintf(stdout, "\nThis concludes the tests\n\n");
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

    fprintf(stdout, "\nThis concludes the tests\n\n");
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
        //fprintf(stdout, 1021, R6, 2, W, X1);
        fprintf(stdout, "The maximum relative error of ");
        xf_print(elemCount, R6, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, " occured for X = ");
        xf_print(elemCount, X1, 4);
        fprintf(stdout, "\n");

        //fprintf(stdout, 1022, 2, W);
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
        //fprintf(stdout, 1023, R7, 2, W);
        fprintf(stdout, "The root mean square relative error was ");
        xf_print(elemCount, R7, 4);
        fprintf(stdout, " = 2^");
        xf_print(elemCount, W, 4);
        fprintf(stdout, "\n");

        //fprintf(stdout, 1022, 2, W);
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
    fprintf(stdout, "The identity exp(X)*exp(-X) = 1.0 will be tested\n");

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

    fprintf(stdout, "\nThis concludes the tests\n\n");
}

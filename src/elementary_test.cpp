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

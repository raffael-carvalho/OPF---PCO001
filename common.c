#include "common.h"

int *AllocIntArray(int n)
{
    int *v = NULL;
    v = (int *)calloc(n, sizeof(int));
    if (v == NULL)
        Error(MSG1, "AllocIntArray");
    return (v);
}

float *AllocFloatArray(int n)
{
    float *v = NULL;
    v = (float *)calloc(n, sizeof(float));
    if (v == NULL)
        Error(MSG1, "AllocFloatArray");
    return (v);
}

void Error(char *msg, char *func)
{ /* It prints error message and exits
                                    the program. */
    fprintf(stderr, "Error:%s in %s\n", msg, func);
    exit(-1);
}

void Warning(char *msg, char *func)
{ /* It prints warning message and
                                       leaves the routine. */
    fprintf(stdout, "Warning:%s in %s\n", msg, func);
}

void Change(int *a, int *b)
{ /* It changes content between a and b */
    const int c = *a;
    *a = *b;
    *b = c;
}

/*
 * The source code to generate random numbers was taken from http://www.physics.drexel.edu/courses/Comp_Phys/Physics-306/random.c.
 */

double ran(int *idum)
{
    int j;
    int k;
    static int idum2 = 123456789;
    static int iy = 0;
    static int iv[NTAB];
    double temp;

    if (*idum <= 0)
    { // *idum < 0 ==> initialize
        if (-(*idum) < 1)
            *idum = 1;
        else
            *idum = -(*idum);
        idum2 = (*idum);

        for (j = NTAB + 7; j >= 0; j--)
        {
            k = (*idum) / IQ1;
            *idum = IA1 * (*idum - k * IQ1) - k * IR1;
            if (*idum < 0)
                *idum += IM1;
            if (j < NTAB)
                iv[j] = *idum;
        }
        iy = iv[0];
    }
    k = (*idum) / IQ1;
    *idum = IA1 * (*idum - k * IQ1) - k * IR1;
    if (*idum < 0)
        *idum += IM1;

    k = idum2 / IQ2;
    idum2 = IA2 * (idum2 - k * IQ2) - k * IR2;
    if (idum2 < 0)
        idum2 += IM2;

    j = iy / NDIV;
    iy = iv[j] - idum2;
    iv[j] = *idum;
    if (iy < 1)
        iy += IMM1;

    if ((temp = AM * iy) > RNMX)
        return RNMX;
    else
        return temp;
}

#undef IM1
#undef IM2
#undef AM
#undef IMM1
#undef IA1
#undef IA2
#undef IQ1
#undef IQ2
#undef IR1
#undef IR2
#undef NTAB
#undef NDIV
#undef EPS
#undef RNMX

static int randx = 0; /* copy of random seed (internal use only) */

/* It initializes the random number generator */
int seedrandinter(int seed)
{
    if (seed == 0)
        seed = (int)time(NULL); /* initialize from the system
						   clock if seed = 0 */
    randx = -abs(seed);
    return seed; /* return seed in case we need to repeat */
}

/* It returns a random integer number uniformly distributed within [low,high] */
int RandomInteger(double low, double high)
{
    if (randx == 0)
        seedrandinter(0);
    return low + (high - low) * ((double)ran(&randx));
}

/* It returns a random float number uniformly distributed within [low,high] */
double RandomFloat(double low, double high)
{
    if (randx == 0)
        seedrandinter(0);
    return low + (high - low) * ((double)ran(&randx));
}

/* It returns a number drawn from a Gaussian distribution
Parameters:
mean: mean of the distribution
variance: variance of the distribution */
double RandomGaussian(double mean, double variance)
{
    double v, x, y, r;

    do
    {
        x = (double)2 * RandomFloat(1, 100) / 99;
        y = (double)2 * RandomFloat(1, 100) / 99;
        r = x * x + y * y;
    } while (r >= 1 || r == 0);

    v = x * sqrt(-2 * log(r) / r) * variance + mean;

    return v;
}

/* It returns a number drawn from Bernoulli distribution with probability p */
int BernoulliDistribution(double p)
{
    double r;
    r = RandomFloat(0, 1);
    if (r < p)
        return 1;
    else
        return 0;
}
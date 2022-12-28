/*
 * statistics.h
 *
 *  Created on: Nov 5, 2022
 *      Author: Sefa Unal
 */

#include <cmath>

#define SCALE_LINEAR(input, inputMin, inputMax, outputMin, outputMax) (((input)-(inputMin))*((outputMax)-(outputMin))/((inputMax)-(inputMin))+(outputMin))

void statistics_range(double *output, int length, double sig_min, double sig_max)
{
    for (int i=0; i<length; i++)
    {
        output[i] = sig_min + i * (sig_max - sig_min) / length;
    }
}

void statistics_pdf(double *input, int inLength, double *range, int rangeLength, double *output)
{
    // clear output buffer
    for (int i = 0; i < rangeLength; i++) output[i] = 0.0;

    // count probabilities
    for (int i = 0; i < inLength; i++)
    {
        for (int j = 0; j < rangeLength; j++)
        {
            if ((input[i] >= range[j]) && (input[i] < range[j + 1]))
            {
                output[j]++;
                break;
            }
        }
    }

    // Normalize
    for (int i = 0; i < rangeLength; i++)
    {
        output[i] = output[i] / inLength;
    }

    // Total should be equal to 1
    double total = 0;
    for (int i = 0; i < rangeLength; i++)
    {
        total += output[i];
    }
    printf("Total: %f\n", total);
}

double statistics_find_index(double *input, int length, double *range, double value)
{
    int i = 0;
    for (; i < length; i++)
    {
        if ((value >= range[i]) && (value < range[i + 1]))
        {
            break;
        }
    }

    return input[i];
}

// Box muller method
// https://stackoverflow.com/questions/19944111/creating-a-gaussian-random-generator-with-a-mean-and-standard-deviation
void rand_init(unsigned int seed)
{
    srand(seed);
}
double rand_normal(double mean, double stddev)
{
    static double n2 = 0.0;
    static int n2_cached = 0;
    if (!n2_cached)
    {
        double x, y, r;
        do
        {
            x = 2.0*rand()/RAND_MAX - 1;
            y = 2.0*rand()/RAND_MAX - 1;

            r = x*x + y*y;
        }
        while (r == 0.0 || r > 1.0);

        {
            double d = sqrt(-2.0*log(r)/r);
            double n1 = x*d;
            n2 = y*d;
            double result = n1*stddev + mean;
            n2_cached = 1;
            return result;
        }
    }
    else
    {
        n2_cached = 0;
        return n2*stddev + mean;
    }
}

void statistics_dist_uniform(double *signal, double length, double min, double max)
{
    for(int i = 0; i < length; i++)
    {
        double randx = (double) rand() / RAND_MAX;
        signal[i] = SCALE_LINEAR(randx, 0.0, 1.0, min, max);
    }
}

void statistics_dist_normal(double *signal, double length, double mean, double Q)
{
    for(int i = 0; i < length; i++)
    {
        signal[i] = rand_normal(mean, Q);
    }
}

void statistics_dist_triangular(double *signal, double length, double lower, double peak, double upper)
{
    for(int i = 0; i < length; i++)
    {
        double randx = (double) rand() / RAND_MAX;
        if (randx < (peak-lower) / (upper-lower))
        {
            signal[i] = lower + sqrt(randx * (upper - lower) * (peak - lower));
        }
        else
        {
            signal[i] = upper - sqrt((1 - randx) * (upper - lower) * (upper - peak));
        }
    }
}


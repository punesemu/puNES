/*
 * filter.c
 *
 * Copyright (c) 2018 Disi A
 * 
 * Author: Disi A
 * Email: adis@live.cn
 *  https://www.mathworks.com/matlabcentral/profile/authors/3734620-disi-a
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "filter.h"
#include <stddef.h>

#if DOUBLE_PRECISION
#define COS cos
#define SIN sin
#define TAN tan
#define COSH cosh
#define SINH sinh
#define SQRT sqrt
#define LOG log
#else
#define COS cosf
#define SIN sinf
#define TAN tanf
#define COSH coshf
#define SINH sinhf
#define SQRT sqrtf
#define LOG logf
#endif

BWLowPass* create_bw_low_pass_filter(int order, FTR_PRECISION s, FTR_PRECISION f) {
    BWLowPass* filter = (BWLowPass *) malloc(sizeof(BWLowPass));
    filter -> n = order/2;
    filter -> A = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    
    if (filter->d2 == NULL)
    {
        free_bw_low_pass(filter);
        return NULL;
    }

    FTR_PRECISION a = TAN((FTR_PRECISION)(M_PI * f / s));
    FTR_PRECISION a2 = a * a;
    FTR_PRECISION r;
    
    int i;
    
    for(i=0; i < filter -> n; ++i){
        r = SIN((FTR_PRECISION)(M_PI * (2.0 * i + 1.0) / (4.0 * filter->n)));
        s = (FTR_PRECISION) (a2 + 2.0 * a * r + 1.0);
        filter->A[i] = a2 / s;
        filter->d1[i] = (FTR_PRECISION) (2.0 * (1 - a2) / s);
        filter->d2[i] = (FTR_PRECISION)(-(a2 - 2.0 * a * r + 1.0) / s);
    }
    return filter;
}

BWHighPass* create_bw_high_pass_filter(int order, FTR_PRECISION s, FTR_PRECISION f){
    BWHighPass* filter = (BWHighPass *) malloc(sizeof(BWHighPass));
    filter -> n = order/2;
    filter -> A = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));

    if (filter->d2 == NULL)
    {
        free_bw_high_pass(filter);
        return NULL;
    }

    FTR_PRECISION a = TAN((FTR_PRECISION) (M_PI * f/s));
    FTR_PRECISION a2 = a * a;
    FTR_PRECISION r;
    
    int i;

    for(i=0; i < filter -> n; ++i){
        r = SIN((FTR_PRECISION)(M_PI * (2.0 * i + 1.0) / ( 4.0 * filter->n)));
        s = (FTR_PRECISION) (a2 + 2.0 * a * r + 1.0);
        filter -> A[i] = (FTR_PRECISION) (1.0 / s);
        filter -> d1[i] = (FTR_PRECISION) (2.0 * (1 - a2) / s);
        filter -> d2[i] = (FTR_PRECISION) (- (a2 - 2.0 * a * r + 1.0) / s);
    }
    return filter;
}

BWBandPass* create_bw_band_pass_filter(int order, FTR_PRECISION s, FTR_PRECISION fl, FTR_PRECISION fu){
    if(fu <= fl){
        printf("ERROR:Lower half-power frequency is smaller than higher half-power frequency");
        return NULL;
    }
    BWBandPass* filter = (BWBandPass *) malloc(sizeof(BWBandPass));
    filter -> n = order/4;
    filter -> A = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d3 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d4 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));

    filter -> w0 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w3 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w4 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));

  
    FTR_PRECISION a = COS(M_PI*(fu+fl)/s)/COS(M_PI*(fu-fl)/s);
    FTR_PRECISION a2 = a*a;
    FTR_PRECISION b = TAN(M_PI*(fu-fl)/s);
    FTR_PRECISION b2 = b*b;
    FTR_PRECISION r;
    int i;
    for(i=0; i<filter->n; ++i){
        r = SIN( (FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / (4.0 * filter->n)));
        s = (FTR_PRECISION)(b2 + 2.0 * b * r + 1.0);
        filter->A[i] = b2/s;
        filter->d1[i] = (FTR_PRECISION) (4.0 * a * (1.0 + b * r) / s);
        filter->d2[i] = (FTR_PRECISION) (2.0 * (b2 - 2.0 * a2 - 1.0)/ s);
        filter->d3[i] = (FTR_PRECISION) (4.0 * a * (1.0 - b * r ) / s);
        filter->d4[i] = (FTR_PRECISION) (- (b2 - 2.0 * b * r + 1.0) / s);
    }
    return filter;
}

BWBandStop* create_bw_band_stop_filter(int order, FTR_PRECISION s, FTR_PRECISION fl, FTR_PRECISION fu){
    if(fu <= fl){
        printf("ERROR:Lower half-power frequency is smaller than higher half-power frequency");
        return NULL;
    }

    BWBandStop* filter = (BWBandStop *) malloc(sizeof(BWBandStop));
    filter -> n = order/4;
    filter -> A = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d3 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));
    filter -> d4 = (FTR_PRECISION *)malloc(filter -> n*sizeof(FTR_PRECISION));

    filter -> w0 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w3 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));
    filter -> w4 = (FTR_PRECISION *)calloc(filter -> n, sizeof(FTR_PRECISION));

    FTR_PRECISION a = COS(M_PI*(fu+fl)/s)/COS(M_PI*(fu-fl)/s);
    FTR_PRECISION a2 = a*a;
    FTR_PRECISION b = TAN(M_PI*(fu-fl)/s);
    FTR_PRECISION b2 = b*b;
    FTR_PRECISION r;

    int i;
    for(i=0; i<filter->n; ++i){
        r = SIN((FTR_PRECISION) (M_PI * (2.0 * i + 1.0 ) / ( 4.0 * filter->n)));
        s = (FTR_PRECISION) (b2 + 2.0 * b * r + 1.0);
        filter->A[i] = (FTR_PRECISION) (1.0 / s);
        filter->d1[i] = (FTR_PRECISION) (4.0 * a * (1.0 + b * r ) / s);
        filter->d2[i] = (FTR_PRECISION) (2.0 * (b2 - 2.0 * a2- 1.0) / s);
        filter->d3[i] = (FTR_PRECISION) (4.0 * a * (1.0 - b * r) / s);
        filter->d4[i] = (FTR_PRECISION) (- (b2 - 2.0 * b * r + 1.0) / s);
    }
    filter->r = (FTR_PRECISION) (4.0 * a);
    filter->s = (FTR_PRECISION) (4.0 * a2 + 2.0);
    return filter;
}

CHELowPass* create_che_low_pass_filter(int n, FTR_PRECISION epsilon, FTR_PRECISION s, FTR_PRECISION f){
    CHELowPass* filter = (CHELowPass *) malloc(sizeof(CHELowPass));
    filter -> m = n/2;
    filter -> A = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));

    if (filter->d2 == NULL)
    {
        free_che_low_pass(filter);
        return NULL;
    }

    FTR_PRECISION a = TAN((FTR_PRECISION) (M_PI * f/ s));
    FTR_PRECISION a2 = a * a;
    
    FTR_PRECISION u = LOG((FTR_PRECISION) (1.0 + SQRT((FTR_PRECISION) (1.0 + epsilon * epsilon)) / epsilon));
    FTR_PRECISION su = SINH(u/(FTR_PRECISION)n);
    FTR_PRECISION cu = COSH(u/(FTR_PRECISION)n);
    FTR_PRECISION b,c;
    
    int i;
    for(i=0; i<filter->m; ++i){
        b = SIN((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / (2.0 * n))) * su;
        c = COS((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / (2.0 * n))) * cu;
        c = b*b + c*c;
        s = (FTR_PRECISION) (a2 * c + 2.0 * a * b + 1.0);
        filter->A[i] = (FTR_PRECISION) (a2 / (4.0 * s));
        filter->d1[i] = (FTR_PRECISION) (2.0 * (1 - a2 * c) / s);
        filter->d2[i] = (FTR_PRECISION) (- (a2 * c - 2.0 * a * b + 1.0) / s);
    }
    filter->ep = (FTR_PRECISION) (2.0 / epsilon);  // used to normalize
    
    return filter;
}

CHEHighPass* create_che_high_pass_filter(int n, FTR_PRECISION epsilon, FTR_PRECISION s, FTR_PRECISION f){
    CHEHighPass* filter = (CHELowPass *) malloc(sizeof(CHEHighPass));
    filter -> m = n/2;
    filter -> A = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));

    if (filter->d2 == NULL)
    {
        free_che_high_pass(filter);
        return NULL;
    }

    FTR_PRECISION a = TAN((FTR_PRECISION) (M_PI * f/ s));
    FTR_PRECISION a2 = a * a;

    FTR_PRECISION u = LOG((FTR_PRECISION) (1.0 + SQRT((FTR_PRECISION) (1.0 + epsilon*epsilon))) / epsilon);
    FTR_PRECISION su = SINH(u/(FTR_PRECISION)n);
    FTR_PRECISION cu = COSH(u/(FTR_PRECISION)n);
    FTR_PRECISION b,c;
    
    int i;
    for(i=0; i<filter->m; ++i){
        b = SIN((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / (2.0 * n))) * su;
        c = COS((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / (2.0 * n))) * cu;
        c = b*b + c*c;
        s = (FTR_PRECISION) (a2 + 2.0 * a * b + c);
        filter->A[i] = (FTR_PRECISION) (1.0 / (4.0 * s));
        filter->d1[i] = (FTR_PRECISION) (2.0 * (c - a2)/s);
        filter->d2[i] = (FTR_PRECISION) (- (a2 - 2.0 * a * b + c) / s);
    }

    filter->ep = (FTR_PRECISION) (2.0 / epsilon); // used to normalize
    return filter;
}

CHEBandPass* create_che_band_pass_filter(int n, FTR_PRECISION epsilon, FTR_PRECISION s, FTR_PRECISION f_lower, FTR_PRECISION f_upper){
    CHEBandPass* filter = (CHEBandPass *) malloc(sizeof(CHEBandPass));
    filter -> m = n/4;
    filter -> A = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d3 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d4 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w3 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w4 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));

    FTR_PRECISION a = COS(M_PI*(f_lower+f_upper)/s)/COS(M_PI*(f_upper-f_lower)/s);
    FTR_PRECISION a2 = a*a;
    FTR_PRECISION b = TAN(M_PI*(f_upper-f_lower)/s);
    FTR_PRECISION b2 = b*b;
    FTR_PRECISION u = LOG((FTR_PRECISION) (1.0 + SQRT((FTR_PRECISION) (1.0 + epsilon * epsilon))) / epsilon);
    FTR_PRECISION su = SINH((FTR_PRECISION) (2.0 * u / (FTR_PRECISION)n));
    FTR_PRECISION cu = COSH((FTR_PRECISION) (2.0 * u / (FTR_PRECISION)n));
    FTR_PRECISION r,c;

    int i;
    for(i=0; i < filter->m; ++i){
        r = SIN((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / n)) * su;
        c = COS((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / n)) * cu;
        c = r*r + c*c;
        s = (FTR_PRECISION) (b2 * c + 2.0 * b * r + 1.0);
        filter->A[i] = (FTR_PRECISION) (b2 / (4.0 * s));
        filter->d1[i] = (FTR_PRECISION) (4.0 * a * (1.0 + b * r) / s);
        filter->d2[i] = (FTR_PRECISION) (2.0 * (b2 * c - 2.0 * a2 - 1.0) / s);
        filter->d3[i] = (FTR_PRECISION) (4.0 * a * (1.0 - b * r) / s);
        filter->d4[i] = (FTR_PRECISION) (- (b2 * c - 2.0 * b * r + 1.0) / s);
    }
    filter->ep = (FTR_PRECISION) (2.0 / epsilon);  // used to normalize
    return filter;
}

CHEBandStop* create_che_band_stop_filter(int n, FTR_PRECISION epsilon, FTR_PRECISION s, FTR_PRECISION f_lower, FTR_PRECISION f_upper){
    CHEBandStop* filter = (CHEBandStop *) malloc(sizeof(CHEBandStop));
    filter -> m = n/4;
    filter -> A = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d1 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d2 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d3 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> d4 = (FTR_PRECISION *)malloc(filter -> m*sizeof(FTR_PRECISION));
    filter -> w0 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w1 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w2 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w3 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));
    filter -> w4 = (FTR_PRECISION *)calloc(filter -> m, sizeof(FTR_PRECISION));

    FTR_PRECISION a = COS(M_PI*(f_lower+f_upper)/s)/COS(M_PI*(f_upper-f_lower)/s);
    FTR_PRECISION a2 = a*a;
    FTR_PRECISION b = TAN(M_PI*(f_upper-f_lower)/s);
    FTR_PRECISION b2 = b*b;
    FTR_PRECISION u = LOG((FTR_PRECISION) (1.0 + SQRT((FTR_PRECISION) (1.0 + epsilon * epsilon))) / epsilon);
    FTR_PRECISION su = SINH((FTR_PRECISION) (2.0*u/(FTR_PRECISION)n));
    FTR_PRECISION cu = COSH((FTR_PRECISION) (2.0*u/(FTR_PRECISION)n));
    FTR_PRECISION r,c;
    int i;
    for(i=0; i < filter->m; ++i){
        r = SIN((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / n)) * su;
        c = COS((FTR_PRECISION) (M_PI * (2.0 * i + 1.0) / n))*cu;
        c = r*r + c*c;
        s = (FTR_PRECISION) (b2 + 2.0 * b * r + c);
        filter->A[i] = (FTR_PRECISION) (1.0 / (4.0 * s));
        filter->d1[i] = (FTR_PRECISION) (4.0 * a * (c + b * r)/s);
        filter->d2[i] = (FTR_PRECISION) (2.0 * (b2 - 2.0 * a2 * c - c) / s);
        filter->d3[i] = (FTR_PRECISION) (4.0 * a * (c - b * r ) / s);
        filter->d4[i] = (FTR_PRECISION) (- (b2 - 2.0 * b * r + c) / s);
    }
    filter->r = (FTR_PRECISION) (4.0 * a);
    filter->s = (FTR_PRECISION) (4.0 * a2 + 2.0);
    filter->ep = (FTR_PRECISION) (2.0 / epsilon);  // used to normalize
    return filter;
}

void free_bw_low_pass(BWLowPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter);
}
void free_bw_high_pass(BWHighPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter);
}
void free_bw_band_pass(BWBandPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> d3);
    free(filter -> d4);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter -> w3);
    free(filter -> w4);
    free(filter);
}
void free_bw_band_stop(BWBandStop* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> d3);
    free(filter -> d4);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter -> w3);
    free(filter -> w4);
    free(filter);
}

void free_che_low_pass(CHELowPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter);
}

void free_che_high_pass(CHEHighPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter);
}
void free_che_band_pass(CHEBandPass* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> d3);
    free(filter -> d4);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter -> w3);
    free(filter -> w4);
    free(filter);
}
void free_che_band_stop(CHEBandStop* filter){
    free(filter -> A);
    free(filter -> d1);
    free(filter -> d2);
    free(filter -> d3);
    free(filter -> d4);
    free(filter -> w0);
    free(filter -> w1);
    free(filter -> w2);
    free(filter -> w3);
    free(filter -> w4);
    free(filter);
}

FTR_PRECISION bw_low_pass(BWLowPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->n; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i] + x;
        x = filter->A[i] * (filter->w0[i] + 2.0f * filter->w1[i] + filter->w2[i]);
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x;
}
FTR_PRECISION bw_high_pass(BWHighPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->n; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i] + x;
        x = filter->A[i] * (filter->w0[i] - 2.0f * filter->w1[i] + filter->w2[i]);
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x;
}
FTR_PRECISION bw_band_pass(BWBandPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->n; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i]+ filter->d3[i]*filter->w3[i]+ filter->d4[i]*filter->w4[i] + x;
        x = filter->A[i] * (filter->w0[i] - 2.0f * filter->w2[i] + filter->w4[i]);
        filter->w4[i] = filter->w3[i];
        filter->w3[i] = filter->w2[i];
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x;
}
FTR_PRECISION bw_band_stop(BWBandStop* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->n; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i]+ filter->d3[i]*filter->w3[i]+ filter->d4[i]*filter->w4[i] + x;
        x = filter->A[i]*(filter->w0[i] - filter->r*filter->w1[i] + filter->s*filter->w2[i]- filter->r*filter->w3[i] + filter->w4[i]);
        filter->w4[i] = filter->w3[i];
        filter->w3[i] = filter->w2[i];
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x;
}

FTR_PRECISION che_low_pass(CHELowPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->m; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i] + x;
        x = filter->A[i] * (filter->w0[i] + 2.0f * filter->w1[i] + filter->w2[i]);
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x * filter->ep;
}
FTR_PRECISION che_high_pass(CHEHighPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->m; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i] + x;
        x = filter->A[i] * (filter->w0[i] - 2.0f * filter->w1[i] + filter->w2[i]);
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x * filter->ep;
}

FTR_PRECISION che_band_pass(CHEBandPass* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->m; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i]+ filter->d3[i]*filter->w3[i]+ filter->d4[i]*filter->w4[i] + x;
        x = filter->A[i] * (filter->w0[i] - 2.0f * filter->w2[i] + filter->w4[i]);
        filter->w4[i] = filter->w3[i];
        filter->w3[i] = filter->w2[i];
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x * filter->ep;
}

FTR_PRECISION che_band_stop(CHEBandStop* filter, FTR_PRECISION x){
    int i;
    for(i=0; i<filter->m; ++i){
        filter->w0[i] = filter->d1[i]*filter->w1[i] + filter->d2[i]*filter->w2[i]+ filter->d3[i]*filter->w3[i]+ filter->d4[i]*filter->w4[i] + x;
        x = filter->A[i]*(filter->w0[i] - filter->r*filter->w1[i] + filter->s*filter->w2[i]- filter->r*filter->w3[i] + filter->w4[i]);
        filter->w4[i] = filter->w3[i];
        filter->w3[i] = filter->w2[i];
        filter->w2[i] = filter->w1[i];
        filter->w1[i] = filter->w0[i];
    }
    return x * filter->ep;
}

FTR_PRECISION softmax(FTR_PRECISION* data, int size, int target_ind){
    FTR_PRECISION sum = 0;
    for(int i = 0; i < size; i++) sum += data[i];
    return data[target_ind]/sum;
}

static const FTR_PRECISION SPIKE_KERNEL[] = {-1.0, 2.0, -1.0};
void spike_filter_upward(FTR_PRECISION * input, int size, FTR_PRECISION * output, FTR_PRECISION strength){
    FTR_PRECISION mean = 0.0;
    FTR_PRECISION std = 0.0;
    FTR_PRECISION diff = 0.0;
    for(int i=0; i < size; i++) mean += input[i];
    mean /= size;

    for(int i=0; i < size; i++){
        diff = input[i] - mean;
        std += diff * diff;
    }
    std = SQRT(std/size);
      
    output[0] = 0.0;
    output[size - 1] = 0.0;
    for(int i=1; i<size-1; i++){
        FTR_PRECISION val = input[i-1] * SPIKE_KERNEL[0] + input[i] * SPIKE_KERNEL[1] + input[i+1] * SPIKE_KERNEL[2];
        if(val < strength * std) output[i] = 0.0;
        else output[i] = val;
    }
}
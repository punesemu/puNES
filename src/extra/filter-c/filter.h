/*
 * filter.h
 *
 * Copyright (c) 2018 Disi A
 * 
 * Author: Disi A
 * Email: adis@live.cn
 *  https://www.mathworks.com/matlabcentral/profile/authors/3734620-disi-a
 */
#ifndef filter_h
#define filter_h

#if __cplusplus
extern "C"{
#endif

#define DOUBLE_PRECISION 0

#if DOUBLE_PRECISION
#define FTR_PRECISION double
#if defined(_WIN32) || defined(__ZEPHYR__)
#define M_PI 3.141592653589793238462643383279502884197163993751
#endif
#else
#define FTR_PRECISION float
#if defined(_WIN32) || defined(__ZEPHYR__)
#ifdef M_PI
#undef M_PI
#define M_PI 3.1415927f
#endif
#endif
#endif

typedef struct {
    int n;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
} BWLowPass;
// BWHighPass uses exactly the same struct
typedef BWLowPass BWHighPass;

typedef struct {
    int m;
    FTR_PRECISION ep;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
} CHELowPass;
typedef CHELowPass CHEHighPass;

typedef struct {
    int n;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *d3;
    FTR_PRECISION *d4;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
    FTR_PRECISION *w3;
    FTR_PRECISION *w4;
} BWBandPass;

typedef struct {
    int m;
    FTR_PRECISION ep;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *d3;
    FTR_PRECISION *d4;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
    FTR_PRECISION *w3;
    FTR_PRECISION *w4;
} CHEBandPass;

typedef struct {
    int n;
    FTR_PRECISION r;
    FTR_PRECISION s;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *d3;
    FTR_PRECISION *d4;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
    FTR_PRECISION *w3;
    FTR_PRECISION *w4;
} BWBandStop;

typedef struct {
    int m;
    FTR_PRECISION ep;
    FTR_PRECISION r;
    FTR_PRECISION s;
	FTR_PRECISION *A;
    FTR_PRECISION *d1;
    FTR_PRECISION *d2;
    FTR_PRECISION *d3;
    FTR_PRECISION *d4;
    FTR_PRECISION *w0;
    FTR_PRECISION *w1;
    FTR_PRECISION *w2;
    FTR_PRECISION *w3;
    FTR_PRECISION *w4;
} CHEBandStop;

BWLowPass* create_bw_low_pass_filter(int order, FTR_PRECISION sampling_frequency, FTR_PRECISION half_power_frequency);
BWHighPass* create_bw_high_pass_filter(int order, FTR_PRECISION sampling_frequency, FTR_PRECISION half_power_frequency);
BWBandPass* create_bw_band_pass_filter(int order, FTR_PRECISION sampling_frequency, FTR_PRECISION lower_half_power_frequency, FTR_PRECISION upper_half_power_frequency);
BWBandStop* create_bw_band_stop_filter(int order, FTR_PRECISION sampling_frequency, FTR_PRECISION lower_half_power_frequency, FTR_PRECISION upper_half_power_frequency);

CHELowPass* create_che_low_pass_filter(int order, FTR_PRECISION epsilon, FTR_PRECISION sampling_frequency, FTR_PRECISION half_power_frequency);
CHEHighPass* create_che_high_pass_filter(int order, FTR_PRECISION epsilon, FTR_PRECISION sampling_frequency, FTR_PRECISION half_power_frequency);
CHEBandPass* create_che_band_pass_filter(int order, FTR_PRECISION epsilon, FTR_PRECISION sampling_frequency, FTR_PRECISION lower_half_power_frequency, FTR_PRECISION upper_half_power_frequency);
CHEBandStop* create_che_band_stop_filter(int order, FTR_PRECISION epsilon, FTR_PRECISION sampling_frequency, FTR_PRECISION lower_half_power_frequency, FTR_PRECISION upper_half_power_frequency);

void free_bw_low_pass(BWLowPass* filter);
void free_bw_high_pass(BWHighPass* filter);
void free_bw_band_pass(BWBandPass* filter);
void free_bw_band_stop(BWBandStop* filter);

void free_che_low_pass(CHELowPass* filter);
void free_che_high_pass(CHEHighPass* filter);
void free_che_band_pass(CHEBandPass* filter);
void free_che_band_stop(CHEBandStop* filter);

FTR_PRECISION bw_low_pass(BWLowPass* filter, FTR_PRECISION input);
FTR_PRECISION bw_high_pass(BWHighPass* filter, FTR_PRECISION input);
FTR_PRECISION bw_band_pass(BWBandPass* filter, FTR_PRECISION input);
FTR_PRECISION bw_band_stop(BWBandStop* filter, FTR_PRECISION input);

FTR_PRECISION che_low_pass(CHELowPass* filter, FTR_PRECISION input);
FTR_PRECISION che_high_pass(CHEHighPass* filter, FTR_PRECISION input);
FTR_PRECISION che_band_pass(CHEBandPass* filter, FTR_PRECISION input);
FTR_PRECISION che_band_stop(CHEBandStop* filter, FTR_PRECISION input);

FTR_PRECISION softmax(FTR_PRECISION* data, int size, int target_ind);

// Output should be pre-allocated which has the same(or larger) size as the input.
void spike_filter_upward(FTR_PRECISION * input, int size, FTR_PRECISION * output, FTR_PRECISION strength);

#if __cplusplus
}
#endif

#endif /* filter_h */
#include "filter.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


void band_pass_example()
{
    BWBandPass* filter = create_bw_band_pass_filter(4, 250, 2, 45);
    
    for(int i = 0; i < 100; i++){
        printf("Output[%d]:%f\n", i, bw_band_pass(filter, i* 100));
    }

    free_bw_band_pass(filter);

}

int main() {   
    printf("========= Band pass filter example =========\n\n");
    band_pass_example();
    printf("========= Done. =========\n\n");
    return 0;
}

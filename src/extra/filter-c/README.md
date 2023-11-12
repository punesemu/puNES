# filter-c

Elegant Butterworth and Chebyshev filter implemented in C, with float/double precision support. Works well on many platforms. You can also use this package in C++ and bridge to many other languages for good performance.

Features
* lowpass
* highpass
* bandpass
* bandstop

This imlementation is based on algorithm from [https://exstrom.com/journal/sigproc/dsigproc.html](https://exstrom.com/journal/sigproc/dsigproc.html)

## Run example
```
make example
./example
```

## Steps to use a filter

1. Create a filter object using `create_***_pass/stop(params...)`


2. Use filter to filter incoming numbers one by one. The output is a double or float that can be specified in header. 


3. After using the filter, release the filter using `free_***_pass/stop(filter)`.

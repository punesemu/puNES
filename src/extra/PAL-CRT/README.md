# PAL-CRT
PAL video signal encoding / decoding emulation by EMMIR 2018-2023
================================================================

### Click the image to see a YouTube video of it running in real time:
[![alt text](/scube_pal.png?raw=true)](https://www.youtube.com/watch?v=7sBli684l5k)
### Click the image to see a YouTube video of it running in real time:
[![alt text](/kc_pal.png?raw=true)](https://www.youtube.com/watch?v=7sBli684l5k)
### Test images
![alt text](/ti_pal.png?raw=true)
### SMPTE Bars (for a better view of the chroma patterns)
![alt text](/cbars_pal.png?raw=true)

### Description
Another deep rabbit hole traversed. PAL is surprisingly more complicated to emulate in software than NTSC.  
This library aims to be a realistic (in terms of visual output) software PAL modem.  
It can be used as a standalone application, an image filter, or used in real time applications like emulators.  
Written to be compatible with C89.  

Just like King's Crook (my from-scratch 3D game), this code follows the same restrictions:

1. Everything must be done in software, no explicit usage of hardware acceleration.
2. No floating point types or literals, everything must be integer only.
3. No 3rd party libraries, only C standard library and OS libraries for window, input, etc.
4. No languages used besides C.
5. No compiler specific features and no SIMD.
6. Single threaded.

------
### Feature List:

- Relatively accurate composite PAL image output  
  -- with bandlimited luma/chroma  
  -- color artifacts  
  -- Hanover bars (with optional suppression of them)  
  -- accurate dot crawl  
  -- the 25 Hz offset of the color subcarrier in standard PAL is emulated  
  
- VSYNC and HSYNC
- Signal noise (optional)
- Interlaced and progressive scan
- Monochrome and full color
- NES decoding support

## Important

The command line program provided does not let you mess with all the settings
like black/white point, brightness, saturation, and contrast.

In the pal_main.c file, there are two main()'s.
One is for a command line program and the other uses my FW library (found here https://github.com/LMP88959/PL3D-KC)
to provide real-time PAL emulation with adjustable parameters.

## Regarding NES Mode

NES Mode is a separate version of PAL-CRT that has adjusted timings to match the non-standard NES specifications.  
https://www.nesdev.org/wiki/PAL_video  
These timings and extra NES specific features were incorporated into the NES version by Persune.  

Special thanks to the NESDev Discord Server and Forums
as well as the the following members (in no particular order):  
 *lidnariq*  
 *Persune*  
 *Eugene.S*  
 *L. Spiro*  
 *org*  
 *feos*  

## Compiling

```sh
cd PAL-CRT

cc -O3 -o pal *.c
```

or using CMake on Linux, macOS, or Windows:

```sh
cmake -B build
cmake --build build
build/pal
```

The default command line takes a single PPM or BMP image file and outputs a processed PPM or BMP file:

```
usage: ./pal -m|o|c|p|r|h|a outwidth outheight noise field phase_error infile outfile
sample usage: ./pal -oc 640 480 24 0 2 in.ppm out.ppm
sample usage: ./pal -pcr 768 576 24 5 0 in.ppm out.ppm
sample usage: ./pal - 832 624 0 2 1 in.ppm out.ppm
-- NOTE: the - after the program name is required
	field number is only meaningful in progressive mode
	phase error is 0, 1, or 2. 0 being none, and 2 being the most
------------------------------------------------------------
	m : monochrome
	o : do not prompt when overwriting files
	c : do Hanover bar correction
	p : progressive scan (rather than interlaced)
	r : raw image (needed for images that use artifact colors)
	a : save analog signal as image instead of decoded image
	h : print help

by default, the image will be full color, interlaced, and scaled to the output dimensions
```

There is also the option of "live" rendering to a video window from an input PPM/BMP image file:

```sh
cmake -B build -Dlive=on
cmake --build build
build/pal my.ppm
```

### Adding PAL-CRT to your C/C++ project:

Global variables:
```c
#include "pal_core.h"

static struct PAL_CRT crt;
static struct PAL_SETTINGS pal;
static int color = 1;
static int noise = 12;
static unsigned field = 0;
static int raw = 0;
static int hue = 0;
```

In your initialization function:
```c
/* pass it the buffer to be drawn on screen */
pal_init(&crt, screen_width, screen_height, PAL_PIX_FORMAT_BGRA, screen_buffer);
/* specify some settings */
crt.blend = 1;
crt.scanlines = 1;
/* check out the PAL_CRT struct for more settings */

```

In your drawing loop:
```c
pal.data = video_buffer; /* buffer from your rendering */
pal.format = PAL_PIX_FORMAT_BGRA;
pal.w = video_width;
pal.h = video_height;
pal.as_color = color;
pal.field = field;
pal.raw = raw;
pal.hue = hue;
/* check out the PAL_SETTINGS struct for more settings */
pal_modulate(&crt, &pal);
pal_demodulate(&crt, noise);
field++;
```

------
If you have any questions feel free to leave a comment on YouTube OR
join the King's Crook Discord server :)

YouTube: https://www.youtube.com/@EMMIR_KC/videos

Discord: https://discord.gg/hdYctSmyQJ

itch.io: https://kingscrook.itch.io/kings-crook

## License
Feel free to use the code in any way you would like, however, if you release anything with it,
a comment in your code/README saying where you got this code would be a nice gesture but it’s not mandatory.

The software is provided "as is", without warranty of any kind, express or implied,
including but not limited to the warranties of merchantability,
fitness for a particular purpose and noninfringement.
In no event shall the authors or copyright holders be liable for any claim,
damages or other liability, whether in an action of contract, tort or otherwise,
arising from, out of or in connection with the software or the use or other dealings in the software.
------
Thank you for your interest!

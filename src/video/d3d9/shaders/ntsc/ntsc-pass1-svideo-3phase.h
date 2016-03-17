{
#include "ntsc-pass1-vertex.inc"
"#define THREE_PHASE\n"
"#define SVIDEO\n"
#include "ntsc-param.inc"
#include "ntsc-rgbyuv.inc"
"\n"
"float4 main_fragment (uniform input IN, in data vertex, uniform sampler2D s0 : TEXUNIT0) : COLOR\n"
"{\n"
#include "ntsc-pass1-encode-demodulate.inc"
"}\n"
"\n"
},

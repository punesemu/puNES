{
"///////////////\n"
"//	TV-out tweaks	\n"
"//	Author: aliaspider - aliaspider@gmail.com\n"
"//	License: GPLv3      \n"
"////////////////////////////////////////////////////////\n"
"\n"
"\n"
"// this shader is meant to be used when running\n"
"// an emulator on a real CRT-TV @240p or @480i\n"
"////////////////////////////////////////////////////////\n"
"// Basic settings:\n"
"\n"
"// signal resolution\n"
"// higher = sharper \n"
"#pragma parameter TVOUT_RESOLUTION \"TVOut Signal Resolution\" 256.0 0.0 1024.0 32.0 // default, minimum, maximum, optional step\n"
"\n"
"// simulate a composite connection instead of RGB\n"
//"#pragma parameter TVOUT_COMPOSITE_CONNECTION \"TVOut Composite Enable\" 0.0 0.0 1.0 1.0\n"
"\n"
"// use TV video color range (16-235) \n"
"// instead of PC full range (0-255)\n"
"#pragma parameter TVOUT_TV_COLOR_LEVELS \"TVOut TV Color Levels Enable\" 0.0 0.0 1.0 1.0\n"
"////////////////////////////////////////////////////////\n"
"\n"
"////////////////////////////////////////////////////////\n"
"// Advanced settings:\n"
"//\n"
"// these values will be used instead \n"
"// if COMPOSITE_CONNECTION is defined\n"
"// to simulate different signal resolutions(bandwidth)\n"
"// for luma (Y) and chroma ( I and Q )\n"
"// this is just an approximation\n"
"// and will only simulate the low bandwidth anspect of\n"
"// composite signal, not the crosstalk between luma and chroma\n"
"// Y = 4MHz I=1.3MHz Q=0.4MHz\n"
"#pragma parameter TVOUT_RESOLUTION_Y \"TVOut Luma (Y) Resolution\" 256.0 0.0 1024.0 32.0\n"
"#pragma parameter TVOUT_RESOLUTION_I \"TVOut Chroma (I) Resolution\" 83.2 0.0 256.0 8.0\n"
"#pragma parameter TVOUT_RESOLUTION_Q \"TVOut Chroma (Q) Resolution\" 25.6 0.0 256.0 8.0\n"
"\n"
"// formula is MHz=resolution*15750Hz\n"
"// 15750Hz being the horizontal Frequency of NTSC\n"
"// (=262.5*60Hz)\n"
"////////////////////////////////////////////////////////\n"
"\n"
"#ifdef PARAMETER_UNIFORM // If the shader implementation understands #pragma parameters, this is defined.\n"
"uniform float TVOUT_RESOLUTION;\n"
//"uniform float TVOUT_COMPOSITE_CONNECTION;\n"
"#define TVOUT_COMPOSITE_CONNECTION 0\n"
"uniform float TVOUT_TV_COLOR_LEVELS;\n"
"uniform float TVOUT_RESOLUTION_Y;\n"
"uniform float TVOUT_RESOLUTION_I;\n"
"uniform float TVOUT_RESOLUTION_Q;\n"
"#else\n"
"// Fallbacks if parameters are not supported.\n"
"#define TVOUT_RESOLUTION 256.0 // Default\n"
"#define TVOUT_COMPOSITE_CONNECTION 0\n"
"#define TVOUT_TV_COLOR_LEVELS 0\n"
"#define TVOUT_RESOLUTION_Y 256.0\n"
"#define TVOUT_RESOLUTION_I 83.2\n"
"#define TVOUT_RESOLUTION_Q 25.6\n"
"#endif\n"
"\n"
"\n"
"struct input\n"
"{\n"
"    half2 video_size;\n"
"    float2 texture_size;\n"
"    half2 output_size;\n"
"};\n"
"\n"
"void main_vertex\n"
"(\n"
"	float4 position	: POSITION ,\n"
"	out float4 outPosition	: POSITION ,\n"
"	float2 texCoord : TEXCOORD0 ,\n"
"	out float2 outTexCoord : TEXCOORD0 ,\n"
"    uniform half4x4 modelViewProj\n"
")\n"
"{\n"
"    outPosition = mul(modelViewProj, position);\n"
"    outTexCoord = texCoord;\n"
" \n"
"}\n"
"float3x3 RGB_to_YIQ = float3x3(\n"
"         0.299,0.587,0.114, \n"
"		 0.595716,-0.274453,-0.321263,\n"
"		 0.211456,-0.522591, 0.311135);\n"
"float3x3 YIQ_to_RGB = float3x3(\n"
"         1.0,0.9563,0.6210, \n"
"		 1.0,-0.2721,-0.6474,\n"
"		 1.0,-1.1070, 1.7046);\n"
"\n"
"\n"
"\n"
"\n"
"#define pi			3.14159265358\n"
"#define a(x) abs(x)\n"
"#define d(x,b) (pi*b*min(a(x)+0.5,1.0/b))\n"
"#define e(x,b) (pi*b*min(max(a(x)-0.5,-1.0/b),1.0/b))\n"
"#define STU(x,b) ((d(x,b)+sin(d(x,b))-e(x,b)-sin(e(x,b)))/(2.0*pi))\n"
"#define X(i) (offset-(i))\n"
"#define L(C) clamp((C -16.5/ 256.0)*256.0/(236.0-16.0),0.0,1.0)\n"
"#define LCHR(C) clamp((C -16.5/ 256.0)*256.0/(240.0-16.0),0.0,1.0)\n"
"\n"
"float3 LEVELS(float3 c0)\n"
"{\n"
"   if (TVOUT_TV_COLOR_LEVELS)\n"
"   {\n"
"      if (TVOUT_COMPOSITE_CONNECTION)\n"
"         return float3(L(c0.x),LCHR(c0.y),LCHR(c0.z));\n"
"      else\n"
"         return L(c0);\n"
"   }\n"
"   else\n"
"      return c0;\n"
"}\n"
"\n"
"\n"
"#define GETC(c) \\\n"
"   if (TVOUT_COMPOSITE_CONNECTION) \\\n"
"      c = mul(RGB_to_YIQ,LEVELS(tex2D(tex, float2(texCoord.x - X*oneT,texCoord.y)).xyz)); \\\n"
"   else \\\n"
"      c = (LEVELS(tex2D(tex, float2(texCoord.x - X*oneT,texCoord.y)).xyz))\n"
"\n"
"#define VAL(tempColor) \\\n"
"   if (TVOUT_COMPOSITE_CONNECTION) \\\n"
"      tempColor += float3((c.x*STU(X,(TVOUT_RESOLUTION_Y*oneI))),(c.y*STU(X,(TVOUT_RESOLUTION_I*oneI))),(c.z*STU(X,(TVOUT_RESOLUTION_Q*oneI)))); \\\n"
"   else \\\n"
"      tempColor += (c*STU(X,(TVOUT_RESOLUTION*oneI)))\n"
"\n"
"float4 main_fragment(in float2 texCoord : TEXCOORD0 , uniform sampler2D tex : TEXUNIT0, uniform input IN) : COLOR\n"
"{\n"
"   float3 tempColor=float3(0.0,0.0,0.0);\n"
"   float	offset	= fract((texCoord.x * IN.texture_size.x) - 0.5);\n"
"   float oneT=1.0/IN.texture_size.x;\n"
"   float oneI=1.0/IN.video_size.x;\n"
"\n"
"   float X;\n"
"   float3 c;\n"
"\n"
"   X = X(-1);\n"
"   GETC(c);\n"
"   VAL(tempColor);\n"
"\n"
"   X = X(0);\n"
"   GETC(c);\n"
"   VAL(tempColor);\n"
"\n"
"   X = X(1);\n"
"   GETC(c);\n"
"   VAL(tempColor);\n"
"\n"
"   X = X(2);\n"
"   GETC(c);\n"
"   VAL(tempColor);\n"
"\n"
"   if (TVOUT_COMPOSITE_CONNECTION)\n"
"      tempColor=mul(YIQ_to_RGB,tempColor);\n"
"   return float4(tempColor, 1.0);\n"
"}\n"
},

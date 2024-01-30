#!/usr/bin/python

import argparse as ap
import os
import fontforge

AP = ap.ArgumentParser()

AP.add_argument('font_file',
	help='TTF file converted from BDF')
AP.add_argument('family_name',
	help='original name of the font')
AP.add_argument('weight',
	help='weight of the font',
	choices=['regular', 'bold'])
AP.add_argument('-i','--italic',
	help='treats the font as an italic variant',
	action='store_true')
AP.add_argument('size',
	help='size in pixels')
AP.add_argument('-o','--output',
	help='where to save the fixed TTF')

args = AP.parse_args()

# ------------ font conversion ------------

if isinstance(args.output, str):
	OFILE = args.output
else:
	OFILE = (
		os.path.splitext(args.font_file)[-2]
	) + '_fixed.ttf'
FFILE = args.font_file
WEIGHT = args.weight.capitalize()
CLASSIFIER = WEIGHT
FAMILY = '_'.join([args.family_name, args.size])
#FAMILY = args.family_name

if args.italic:
	CLASSIFIER += ' Italic'

stylemap = {
	"Regular": 64,
	"Regular Italic": 1,
	"Bold": 32,
	"Bold Italic": 33
}

hfont = fontforge.open(FFILE)
hfont.reencode('iso10646-1')

frname = 'Italic' if CLASSIFIER == 'Regular Italic' else CLASSIFIER

hfont.sfnt_names = ()
hfont.sfnt_names = hfont.sfnt_names + (('English (US)', 'SubFamily', frname), )
	
hfont.fontname = f"{FAMILY.replace(' ','')}-{frname.replace(' ','')}"
hfont.familyname = FAMILY
hfont.weight = WEIGHT
hfont.fullname = f'{FAMILY} {CLASSIFIER}'
hfont.os2_stylemap = stylemap[CLASSIFIER]

hfont.generate(OFILE)

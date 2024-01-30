// pcf -> bdf
pcf2bdf t0-11-uni.pcf > t0-11-uni.bdf
// bdf -> ttf
java -jar BitsNPicas.jar convertbitmap -f ttf -o t0-11-uni.ttf t0-11-uni.bdf
# python fix-ttf.py [fontfile].ttf '[fontname]' [regular/bold] [fontsize]
python fix-ttf.py t0-11-uni.ttf 'TType0' regular 11


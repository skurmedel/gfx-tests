cc -O2 -Wall -std=c99 sobel.c tga.c -o sobel
cat mandrill_24bpp.tga | sobel > sobel_out.tga
rm sobel sobel.exe
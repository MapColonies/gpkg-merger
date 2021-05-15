#!/bin/bash
gcc -o wand wand.c `pkg-config --cflags --libs MagickWand`
./wand ../../../ImageMagick-7.0.11/logo.png lol.png
#!/bin/sh

# Any subsequent commands which fail will cause the shell script to exit immediately
set -e

if [ ! -e data1.fbz ]
then
    echo "Cannot find data1.fbz, this script must be run inside the shadowgrounds installation dir."
    exit 1
fi

#if [ -e data4.fbz ]
#then
#    echo "Fix file data4.fbz already exists. If it was created by this script remove it first"
#    exit 1
#fi

TMPDIR=`mktemp -d`
SRCDIR=`pwd`
cd "${TMPDIR}"
#extracting the needed files and renaming them is easier than replacing the source where they are referenced

#unzip all the needed files
unzip -j "${SRCDIR}/data1.fbz" Data/Effects/Particles/flamethrower_part1.txt Data/Effects/Particles/flamethrower_part2.txt Data/Effects/Particles/flamethrower_part3.txt Data/Textures/Particles/Fluids/splash.tga


#flamethrow_part*_low are missing (just reuse the normal ones)
mkdir -p data/effects/particles
mv flamethrower_part1.txt data/effects/particles/flamethrower_part1_low.txt
mv flamethrower_part2.txt data/effects/particles/flamethrower_part2_low.txt
mv flamethrower_part3.txt data/effects/particles/flamethrower_part3_low.txt

#Data/Textures/Particles/Fluids/awter_splash.tga is referenced, i guess awter is a typo for water and the needed file should be Data/Textures/Particles/Fluids/splash.tga
mkdir -p data/textures/particles/fluids
mv splash.tga data/textures/particles/fluids/awter_splash.tga

#data/models/particles/emitter_shapes/soapy.tga is missing, I found it in the JackClaw zip
#It is just a fully transparent white 16x16 tga image, create it using ImageMagick 
mkdir -p data/models/particles/emitter_shapes
convert -size 16x16 'xc:rgba(255, 255, 255, 0)' data/models/particles/emitter_shapes/soapy.tga

#create an update zip
zip -r data4.fbz data
cp -iv data4.fbz "$SRCDIR"
rm -rv "$TMPDIR"

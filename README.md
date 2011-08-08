SHADOWGROUNDS
============

SHADOWGROUNDS is an epic action experience combining modern technology with addictive playability.

* Devastating weapons and extensive weapon upgrades
* Realistic lighting effects
* 11 exciting missions taking place on various battlegrounds
* Thrilling sound effects and acclaimed soundtrack
* Old-school attitude, modern graphics (including the awesome flamethrower!)


Notes
-----

Shadowgrounds specific code is under /shadowgrounds, and Shadowgrounds Survivor under /survivor.<br/>
Shadowgrounds does not use PhysX, but Shadowgrounds Survivor does.<br/>
PhysX SDK can be obtained from NVIDIA - see http://developer.nvidia.com/physx/ or
http://supportcenteronline.com/ics/support/default.asp?deptID=1949<br/>
More PhysX help: http://physxinfo.com/news/901/how-to-register-developer-account-to-get-physx-sdk-access/
and http://knol.google.com/k/introduction-to-the-nvidia-physx-api<br/>

To get the game assets, you need a copy of each game.


Building
--------

Only 64-bit Linux version is being developed/supported by now, however Shadowgrounds are supposed to be working on Windows/Linux/Mac. Any assistance is welcomed.

External dependencies:

* Boost 1.37
* SDL (core, sound, image, ttf)
* OpenGL
* OpenAL
* GTK2
* ZLIB

PhysX SDK isn't supported yet.

Checkouting and building is simple (Linux):

    git clone git://github.com/vayerx/shadowgrounds.git
    mkdir shadowgrounds/build
    cd shadowgrounds/build
    cmake -DCMAKE_BUILD_TYPE=Release ..
    gmake

Data-files (`data1.fbz`, `data/` for Shadowgrounds) should be located at the current working directory.

Alternative (original) build:

    # need to create libunzip.a
    # once only
    # should integrate to build system
    # but unzip is old shit C code and g++ barfs on it
    cd filesystem/detail
    gcc -c -o ioapi.o -O ioapi.c
    gcc -c -o unzip.o -O unzip.c
    ar r libunzip.a ioapi.o unzip.o
    ranlib libunzip.a
    mv libunzip.a ../../binaries/
    cd ../..

    cd binaries
    cp example.mk local.mk
    # Edit local.mk to suit your tastes.

    # Create necessary subdirectories. Only needed the first time and when directory structure changes
    make bindirs

    # The build system is parallel build safe, add -j<n> if you like
    make


Make-only build won't be supported in favor of Cmake.


Special Notes
-------------

The Shadowgrounds games are quite old and possibly very messy - these sources have not been cleaned too much.<br/>
There may also be references to older projects, such as "Disposable" or "DH", which mean our very first game,
an (RTS) prototype from 2001-2003. (It should be possible to enable several RTS commands such as unit groups,
switching sides to aliens and so on.)<br/>
It's also worth noting that the source code has a lot of "hacks" and "todos". It is in many ways messy and
could be hard to understand.<br/>
We haven't had a commercial name for the game engine, but internally it's been called "Storm3D". Storm3D is
partially based on work by the awesome Finnish programmer Sebastian Aaltonen, from whom we bought the original
code in 2000/2001.

Good luck! :)


Thanks
------

Thanks to all of the Frozenbyte / Shadowgrounds / Shadowgrounds Survivor team(s).<br/>
Thanks to Alternative Games for the Linux/Mac versions<br/>
Thanks to Parallax Software for license inspiration.<br/>

Frozenbyte team - 2011<br/>
http://www.frozenbyte.com/<br/>
http://www.shadowgroundsgame.com/<br/>
http://www.shadowgroundssurvivor.com/

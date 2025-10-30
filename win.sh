#!/bin/bash
make -f makefile.m32 clean
make -f makefile.m32 clean winx=WIN64
make -f makefile.m32
make -f makefile.m32 winx=WIN64
make -f makefile.m32 dist
make -f makefile.m32 winx=WIN64 dist


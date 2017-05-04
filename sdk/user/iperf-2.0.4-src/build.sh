#!/bin/sh
#

#export CONFIG_SITE=config.site
export PATH+=:/rsdk-1.3.6-5181-EB-2.6.30-0.9.30/bin
export CC=rsdk-linux-gcc
export LD=rsdk-linux-gcc
#export CXX=rsdk-linux-g++
export CXX=rsdk-linux-c++
#export CFLAGS="-march=5181 -EL"
#export LDFLAGS="-march=5181 -EL"
#export CXXFLAGS=
export LDFLAGS="-static"
export ac_cv_func_malloc_0_nonnull=yes

./configure --host=mips-linux \
            --disable-threads \
            --disable-web100
            

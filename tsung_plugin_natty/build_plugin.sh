#!/bin/bash

cp tsung-1.0.dtd $1/
cp include/ts_natty.hrl $1/include/
cp src/tsung_controller/ts_config_natty.erl $1/src/tsung_controller/
cp src/tsung/ts_natty.erl $1/src/tsung/
cp natty.xml* $1/examples/

cd $1/
./configure --prefix=/usr/local
sudo make install

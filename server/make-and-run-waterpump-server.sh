#!/bin/bash

export LD_LIBRARY_PATH=$HOME/Qt/5.15.2/gcc_64/lib/:$LD_LIBRARY_PATH
cd qtopcua/build && make sub-examples -j8 && examples/opcua/waterpump/simulationserver/simulationserver

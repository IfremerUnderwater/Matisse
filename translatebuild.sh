#!/bin/bash

PROJECTDIR=.

cd $PROJECTDIR/Src/Libraries/MatisseTools
lupdate ./MatisseTools.pro
cd ../../..
pwd

cd $PROJECTDIR/Src/MatisseServer
lupdate ./MatisseServer.pro
cd ../..
pwd

cd $PROJECTDIR/Src/Libraries/MatisseTools
lrelease ./MatisseTools.pro
cd ../../..
pwd

cd $PROJECTDIR/Src/MatisseServer
lrelease ./MatisseServer.pro
cd ..
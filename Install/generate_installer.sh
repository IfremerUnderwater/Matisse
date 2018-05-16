#!/bin/bash
# This script generates the matisse installer. It has to be executed in its directory

# remove any residual data
rm -rf ./packages/Matisse3D/data

# copy new data
cp -r ../Run/Release ./packages/Matisse3D/data

# generate installer
binarycreator.exe --offline-only -c config/config.xml -p packages matisse-setup.exe

# remove data
rm -rf ./packages/Matisse3D/data
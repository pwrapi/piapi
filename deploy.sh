#!/bin/sh

IMAGE_PATH=/tftpboot/images/compute_powerinsight/usr
#IMAGE_PATH=/tftpboot/images/compute.x86_64/usr
#IMAGE_PATH=/usr

echo "Copying include files to $IMAGE_PATH"
cp ./piapi.h $IMAGE_PATH/include
cp ./picommon.h $IMAGE_PATH/include

echo "Copying libraries to $IMAGE_PATH"
cp ./libpiapi.so $IMAGE_PATH/lib
cp ./libpiapi.so $IMAGE_PATH/lib64

echo "Copying executables to $IMAGE_PATH"
cp ./piproxy $IMAGE_PATH/bin
cp ./pilogger $IMAGE_PATH/bin


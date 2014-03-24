#!/bin/sh

IMAGE_PATH=/tftpboot/images/compute.x86_64/usr

echo "Copying include files to $IMAGE_PATH"
cp ./piapi.h $IMAGE_PATH/include
cp ./picommon.h $IMAGE_PATH/include

echo "Copying library to $IMAGE_PATH"
cp ./libpiapi.so $IMAGE_PATH/lib

echo "Copying executables to $IMAGE_PATH"
cp ./piproxy $IMAGE_PATH/bin
cp ./pilogger $IMAGE_PATH/bin


#!/bin/sh

for IMAGE_PATH in '/images/ro-rootfs/n36_NFSroot_ubuntu_MAIN/usr' '/usr'
do
	echo "Copying include files to $IMAGE_PATH"
	cp ./piapi.h $IMAGE_PATH/include
	cp ./picommon.h $IMAGE_PATH/include

	echo "Copying libraries to $IMAGE_PATH"
	cp ./libpiapi.so $IMAGE_PATH/lib
	cp ./libpiapi.so $IMAGE_PATH/lib64

	echo "Copying executables to $IMAGE_PATH"
	cp ./piproxy $IMAGE_PATH/bin
	cp ./pilogger $IMAGE_PATH/bin
done

for IMAGE_PATH in '/images/ro-rootfs/PI-2014-10-08.nfs/rootfs/home'
do
	echo "Extracting beaglebone files to $IMAGE_PATH"
	rm -rf $IMAGE_PATH/power
	tar zxpf /home/ddeboni/archive/kratos_tools_arm.tgz

	echo "Moving beaglebone files to $IMAGE_PATH"
	mv install $IMAGE_PATH/power
done

#!/bin/sh

for IMAGE_PATH in '/tftpboot/images/compute_powerinsight/usr' '/tftpboot/images/compute.x86_64/usr' '/usr'
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

for IMAGE_PATH in '/tftpboot/images/ro-rootfs/home' '/tftpboot/images/ro-rootfs-alt/home'
do
	echo "Extracting beaglebone files to $IMAGE_PATH"
	rm -rf $IMAGE_PATH/power
	tar zxpf /home/ddeboni/archive/kratos_tools_arm.tgz

	echo "Linking configuration file to $IMAGE_PATH"
	if [ "$IMAGE_PATH" == "/tftpboot/images/ro-rootfs/home" ]
	then
		ln -s config.amd build/bin/.config
	else
		ln -s config.intel build/bin/.config
	fi

	echo "Moving beaglebone files to $IMAGE_PATH"
	mv build $IMAGE_PATH/power
	ln -s $IMAGE_PATH/../etc/powerinsight.conf $IMAGE_PATH/power/bin/.config
done


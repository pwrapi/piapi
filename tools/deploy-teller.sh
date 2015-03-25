#!/bin/sh

PIVER = 1

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

	echo "Moving beaglebone files to $IMAGE_PATH"
	mv install $IMAGE_PATH/power
done

echo "Creating configuration links"
if [ "$PIVER" = "2" ] ; then
	cd /tftpboot/images/ro-rootfs/home/power/bin
	ln -s config.amd pidev.conf

	cd /tftpboot/images/ro-rootfs-alt/home/power/bin
	ln -s config.intel pidev.conf
else
	cd /tftpboot/images/ro-rootfs/etc
	ln -s ../home/power/bin/config.amd powerinsight.conf

	cd /tftpboot/images/ro-rootfs-alt/etc
	ln -s ../home/power/bin/config.intel powerinsight.conf
fi

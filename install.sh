#!/bin/bash

#get current directory
CAMERA_DIR=`pwd`

#install dependencies
sudo apt install build-essential cmake libgtk-3-dev libopencv-dev pkg-config

# debian package version of libserialport doesn't work on kernels >5.10
# so for modern ones we need the more recently patched repo version
sudo apt remove libserialport0 libserialport-dev
pushd .
cd
git clone https://github.com/sigrokproject/libserialport
cd libserialport
./autogen.sh
./configure
make
sudo make install
popd


#prepare makefile
cmake .

#build
make

#provide startup scripts
mkdir $HOME/bin

echo "cd $CAMERA_DIR/bin
./config" > $HOME/bin/config.sh

echo "cd $CAMERA_DIR/bin
./rollercoaster" > $HOME/bin/rollercoaster.sh

chmod 740 $HOME/bin/config.sh
chmod 740 $HOME/bin/rollercoaster.sh

#provide desktop shortcuts
echo "[Desktop Entry]
Version=1.0
Type=Application
Terminal=true
Exec=$HOME/bin/config.sh
Name=Configurator
Comment=
Icon=$CAMERA_DIR/bin/icon.png" > $HOME/Desktop/Configurator.desktop

echo "[Desktop Entry]
Version=1.0
Type=Application
Terminal=true
Exec=$HOME/bin/rollercoaster.sh
Name=Coaster
Comment=
Icon=$CAMERA_DIR/bin/icon.png" > $HOME/Desktop/Coaster.desktop

# mark desktop shortcuts executable
chmod 740 $HOME/Desktop/Configurator.desktop
chmod 740 $HOME/Desktop/Coaster.desktop

# add coaster to startup
mkdir $HOME/.config/autostart
cp $HOME/Desktop/Coaster.desktop $HOME/.config/autostart/

# set desktop shortcuts as trusted
#gio set $HOME/Desktop/Configurator.desktop "metadata::trusted" yes
#gio set $HOME/Desktop/Coaster.desktop "metadata::trusted" yes

# add user to "dialout"
SELF=`whoami`
sudo usermod -a $SELF -G dialout

echo ""
echo "|---------------------------------------|"
echo "| Installation complete; please reboot! |"
echo "|---------------------------------------|"
echo ""

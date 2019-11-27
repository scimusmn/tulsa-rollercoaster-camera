#!/bin/bash

#get current directory
CAMERA_DIR=`pwd`

#install dependencies
sudo apt install build-essential cmake libgtk-3-dev libopencv-dev libserialport-dev pkg-config

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

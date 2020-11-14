#!/bin/bash
apt install v4l2loopback-dkms
echo "options v4l2loopback devices=1 video_nr=20 card_label="bgcam" exclusive_caps=1" > /etc/modprobe.d/bgcam.conf
echo v4l2loopback > /etc/modules-load.d/bgcam.conf
modprobe -r v4l2loopback
modprobe v4l2loopback
chmod +x build/bg.sh

# linux-camera-background
Blurs your background for video conferencing on linux. 

# How it works
This program uses lbp cascades to find face(s) in the video feed, then cuts out the section of the image containing the face(s), stores that section, blurs the original image, and superimposes the cut section on top of the blurred image, then sends the edited frames to another video device.

# Install
Debian and deb-based systems:

Clone the repo:
```
git clone https://github.com/varun-dhar/linux-camera-background.git
```

Run `install.sh`:
```
chmod +x install.sh
sudo ./install.sh
```
Or do it manually:

Install the v4l2loopback kernel module:
```
sudo apt install v4l2loopback-dkms
```
Add config for v4l2loopback:
```
echo "options v4l2loopback devices=1 video_nr=20 card_label="bgcam" exclusive_caps=1" | sudo tee -a /etc/modprobe.d/bgcam.conf
```
You can also manually create a .conf file in /etc/modprobe.d with those options and/or change the card_label (what the device appears as) or video_nr (the device number, e.g. /dev/video1 for device number 1) fields. Changing video_nr is not recommended, as you will also have to set pyfakewebcam to use it in the script. 

```
echo v4l2loopback | sudo tee -a /etc/modules-load.d/bgcam.conf
```
Again, the .conf file can be called whatever you want it to be called.

Reload v4l2loopback kernel module:
```
sudo modprobe -r v4l2loopback
sudo modprobe v4l2loopback
```
Install dependencies:
(I'm assuming you have a functional python3 environment with pip)
```
python3 -m pip install numpy opencv-contrib-python pyfakewebcam
```

Other systems:

Install the `v4l2loopback` module using whatever package manager you use, then follow the same steps from "Add config for v4l2loopback".

# Usage
To use the program, run it with `./bg.py`. The new webcam device will show up in your conferencing application as whatever card_label is in the v4l2loopback config (default bgcam). In options.txt, one can configure the offsets for the size of the cutout and its position on the screen. You can also enable and disable the rectangle that marks the cutout to help with the adjustment process by inputting a 0 for disabled and 1 for enabled.
# Side notes
There's an issue where the virtual webcam device can't be found unless using a Chromium-based application. If you experience this, check for updates to v4l2loopback and your kernel, and if available, install them. If not, try restarting. Theres another issue where the virtual webcam can't be found in MS Teams. This is likely a Teams issue; though this is not confirmed. I would submit a bug report, but if I know anything about MS's maintenance team, it's that they never listen to their users(one of the reasons I made this). However, this is my personal opinion (which doesn't matter), so feel free to submit one yourself.

New features coming soon: Filter that makes you look like a comic. Example shown [here](https://i.ibb.co/f87sHS8/out.jpg) (using a CC image I found by googling "person").

The program does use a lot of CPU resources so expect slowdowns if you have other CPU-hungry programs open. The video may stutter slightly if you move your head a lot (the classifier is kinda slow and cant handle rapid changes). C++ rewrite has been completed and is on the `cpp-rewrite` branch; it's more resource-efficient, has a higher framerate and weirldy, is much larger. Contributions are welcome and appreciated. Feature requests may be submitted on the issues page. 

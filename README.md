# linux-camera-background
Blurs or substitutes your background for video conferencing on linux. 

# How it works
This program uses TensorFlow Bodypix to find a person in the video feed, then creates an image mask of the shape of the person's body, then uses that mask to determine which section of the image to blur/replace with an image, then sends the edited frames to a virtual webcam device.

# Install
Debian and deb-based systems:

Clone the repo:
```
git clone https://github.com/varun-dhar/linux-camera-background.git
cd linux-camera-background
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
python3 -m pip install -r requirements.txt
```

Other systems:

Install the `v4l2loopback` module using whatever package manager you use, then follow the same steps from "Add config for v4l2loopback".

# Usage
To use the program, run it with `./bg.py`. The new webcam device will show up in your conferencing application as whatever card_label is in the v4l2loopback config (default bgcam). In options.txt, one can configure how much the program blurs your background, choose an image to use as the background, and change the sensitivity of the mask, so if it cuts off too much or shows too much, you can fix it.

# Config file
`blurFactor`- value from 0-100 determining how much you want to blur the background.

`useImageBg`- value from 0-1 determining whether or not you want to use a background image file. 0 is false, 1 is true.

`backgroundFile`- path to background image you want to use. Optional, doesn't do anything if `useImageBg` is 0.

`maskSensitivity`- value from 0-100 determining how sensitive the mask should be. Decrease if parts of your face/body are cut off, increase if there are random blotches of (original) background appearing.

# Side notes
There's an issue where the virtual webcam device can't be found unless using a Chromium-based application. If you experience this, check for updates to v4l2loopback and your kernel, and if available, install them. If not, try restarting. Theres another issue where the virtual webcam can't be found in MS Teams. This is likely a Teams issue; though this is not confirmed. I would submit a bug report, but if I know anything about MS's maintenance team, it's that they never listen to their users(one of the reasons I made this). However, this is my personal opinion (which doesn't matter), so feel free to submit one yourself. Current workaround is to use the web client or the unofficial [Electron client](https://github.com/IsmaelMartinez/teams-for-linux).

New features coming soon: Filters! (when I get around to it).

The program does use a lot of CPU resources so expect slowdowns if you have other CPU-hungry programs open. However, it ran fine on my mid range 4yo laptop with ~6 resource-intensive windows open, so it should be fine. The video may stutter slightly if you move your head a lot (the model is kinda slow if you don't have good specs and cant handle rapid changes). I might do a C++ rewrite of this code, but probably not, as there is little documentation on using TensorFlow Bodypix in C++. C++ rewrite of the version on the `python-old` branch has been completed and is on the `cpp-rewrite` branch; it's more resource-efficient, has a higher framerate and weirldy, is much larger. Contributions are welcome and appreciated. Feature requests may be submitted on the issues page. 

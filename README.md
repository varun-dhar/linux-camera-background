# linux-camera-background
Blurs your background on linux for video conferencing. 

# How it works
This program uses lbp cascades to find face(s) in the video feed, then cuts out the section of the image containing the face(s), stores that section, blurs the original image, and superimposes the cut section on top of the blurred image, then sends the edited frames to another video device.
# Install
Debian and deb-based systems:

Clone the repo:

```
git clone -b "cpp-rewrite" https://github.com/varun-dhar/linux-camera-background.git
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
Make the runner script executable:
```
chmod +x build/bg.sh
```
# Usage
To use the program, cd to the build directory and run `./bg.sh`. The new webcam device will show up in your conferencing application as whatever card_label is in the v4l2loopback config (default bgcam). In options.txt, one can configure the offsets for the size of the cutout and its position on the screen. You can also enable and disable the rectangle that marks the cutout to help with the adjustment process by inputting a 0 for disabled and 1 for enabled. The blur factor is also adjustable in options.txt (side note: it must be an odd number).
# Side notes
As one can see, I did the rewrite. It's still cpu-hungry, but less so, and the framerate is higher. A static build was not possible, and I did not want to force the installation of the many packages required, so all the necessary .so files are in `build/lib`. Not quite sure about this, but ironically the executable + the .so files seem to have a larger total filesize than the python version (with its dependencies!). Who would have thought that the C++ version would be the mega thicc one? It works well in WebRTC and video conferencing apps, however reading from the virtual webcam with `ffplay` takes persistence. If it cannot find your face, it just blurs the whole image. Mad props to OscarAcena for his [article](https://arcoresearchgroup.wordpress.com/2020/06/02/virtual-camera-for-opencv-using-v4l2loopback/) on using v4l2loopback with OpenCV in C++.

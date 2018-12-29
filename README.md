# ![QCCTV](etc/qcctv.png)

QCCTV allows you to use your devices (computers, mobile phones, tablets, etc.) to implement a CCTV network in your LAN.

Besides security, QCCTV can have many uses, such as recording a video with multiple cameras at once or tell if your cat is plotting to kill you.

The QCCTV suite consists of two applications:
- **QCCTV Camera**, which streams your camera's live images to the LAN
- **QCCTV Station**, which receives camera streams from the LAN and manages each camera individually

### How QCCTV works

- QCCTV Cameras broadcast a small UDP packet periodically to make themselves visible to any QCCTV Station in the same network.
- Once the UDP packet is received, the QCCTV Station will attempt to establish a TCP connection with the camera
- Once the TCP connection is established, the camera will send these packets periodically:
	- Binary JSON data containing camera status and information (with an UDP socket)
	- Compressed camera frame (in JPEG format) and CRC32 checksum of the data (with a TCP socket)
- On the other hand, the QCCTV Station will respond to camera packets with the following data:
	- Current FPS and desired FPS
	- Current resolution and desired resolution
	- Current zoom and desired zoom
	- Current flash light status and desired flashlight status
	- Focus request byte (if applicable)
	- Current and desired values are sent in order to avoid overwritting any configuration set locally by the camera; If the current (or "old") value in the packet does not correspond to the current value used by the camera, then the QCCTV Camera shall ignore the request
- If allowed, the QCCTV Camera shall auto-regulate its resolution to improve communication speeds. This option can be configured remotely by the QCCTV Station or locally, by the camera itself

### Networking Code

- The ports used by QCCTV are set in [this header](https://github.com/alex-spataru/qcctv/blob/master/common/src/QCCTV.h#L33)
- All the stream encoding/decoding code is found in:
	- [QCCTV_Communications.h](https://github.com/alex-spataru/qcctv/blob/master/common/src/QCCTV_Communications.h)
	- [QCCTV_Communications.cpp](https://github.com/alex-spataru/qcctv/blob/master/common/src/QCCTV_Communications.cpp)

### Icons 

The icons come from the "Global Security" icon set from Aha-Soft and are released under the Creative Commons license.

You can download the icon set from [http://aha-soft.com](http://www.aha-soft.com/free-icons/free-global-security-icons/)

### License

The QCCTV software suite is released under the MIT license.

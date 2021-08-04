# QSoftscopeGPIB
Qt application to control my HP 54540A oscilloscope over the GPIB bus using the linux-gpib library. Waveforms are displayed on a QWT plot.

![qsoftscopegpib](https://user-images.githubusercontent.com/26282917/128179809-73a4975a-43f7-43fa-88c0-ae81c8fdfa0f.png)

# Prerequisites
- Qt
- linux-gpib
- Qwt
- GPIB-USB Adapter

# Compatibility
May work with other HP **digitizing** oscilloscopes from around that time (~1994) if you adjust the maximum sample rate and the number of channels.
Should work perfectly with the HP 54542A, 54540C, 54542C, 54520A, 54520C, 54522A, 54522C.

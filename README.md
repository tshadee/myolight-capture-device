# MYOLIGHT™ General Purpose Electromyogram Capture Device

## Overview

MYOLIGHT™ is system of two components: a HDsEMG device featuring 32 electrodes and 28 single-differential channels, and a Python software package which provides live FFT visualisation of those 28 SD channels.

This repository contains code for the embedded front-end integrated with PlatformIO and Python software for the MYOLIGHT™ system. 

## Prerequisites

The HDsEMG device runs on a [Xiao ESP32C6](https://www.seeedstudio.com/Seeed-Studio-XIAO-ESP32C6-p-5884.html) which requires the [PlatformIO extension](https://platformio.org/) to run with Microsoft Visual Studio Code.

Installation instructions for PlatformIO can be found on their website, or installed via the `extensions` tab on the left of VSC.  

The ESP32C6 is currently not supported as a device on PlatformIO. Please follow Seeed Studio's [instructions](https://wiki.seeedstudio.com/xiao_esp32c6_with_platform_io/) for how to use ESP32C6 on PIO.

## Installation (HDsEMG embedded software):

The current repository you are in can be cloned into the project folder created by PlatformIO. You may need to adjust the `platformio.ini` file to update the ESP32C6 drivers to its latest versions for compatibility,
but otherwise, compile and burn it onto the microcontroller! No extra setup is required.

_TL;DR_: clone, compile, and burn onto the ESP32C6! (make sure to follow instructions in the `prerequisite`)

## Installation (Python Software):

If you have cloned the repository, there will be a `/PythonScripts/` folder containing various files. 

Feel free to move it somewhere else; this is not required in the compilation process for the embedded software.

### Average User
Go into `/Python Scripts/dist` and you will find a `myolight.exe` file. Download the file for MYOLIGHT™'s latest stable build.

### Power User
A virtual environment is recommended before starting. 

Run `pip install -r requirements.txt` to install dependencies and launch the program using `myolight.py` with `python myolight.py`. 

If changes are made to the python script and you want to rebuild the executable, run the `build_exe.bat` file. This will automatically compile and generate a new `myolight.exe` inside the `/dist` folder.

Note that the GUI uses `CTkinter` and is currently incompatible with Python 3.12.xx unless you modify the `PATH` variable.

## Usage



1. Begin by powering on the microcontroller (battery switch to the `ON` and `2L` state) on the embedded device and connect to the Wi-Fi soft access point `ESP32C6T-softAP` with the password `qw12er34`.

   Do not fret, this password can be changed (if you are a power user). Look in `myolight-capture-device/include/COMMON_DEFS.h` and there will be a `#DEFINE WIFI_PASSWORD "xxxxxx"`. The MYOLIGHT™ uses WPA2PSK encryption, so all your biodata is definitely safe with us👍.

2. On successful connection to the `ESP32C6T-softAP` Wi-Fi, launch `myolight.exe`. This will bring up the interface:

   [![Screenshot-2025-04-10-225900.png](https://i.postimg.cc/43cmxjv4/Screenshot-2025-04-10-225900.png)](https://postimg.cc/f3wwH2g1)

   (The buttons are self-explanatory. They're even disabled for you not to randomly click and break things!)

3. Start with `Search and Connect`. If the terminal outputs an `[ECHO]` message, you are good to go. If not, unlucky (to be fixed).
4. Configure sampling parameters before hitting `Send Config`. An `[ECHO]` will indicate the successful reconfiguration of the ADC and internal flags.
5. Begin data collection with... `Start Data Collection`. This will launch another window:

   [![Screenshot-2025-04-09-213903.png](https://i.postimg.cc/hGsNTt12/Screenshot-2025-04-09-213903.png)](https://postimg.cc/zLLd15Mh)

   This is the FFT display of all 28 channels. Touch the electrode array to get a sense of electrode mapping. You'll get the hang of it, trust 😎.

6. Data collection can be stopped with `Stop Data Collection` which will...stop data collection!
7. `Analyse Data` can be ignored (in development).

## Bugs and Issues

See [Issues](https://github.com/tshadee/myolight-capture-device/issues) (may not be addressed).














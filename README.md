# GBS8200-reprog

or a bunch of tools for dumping and reprogramming the MTV230M microcontroller, found on the GBS8200 scaler board
Naming subject to change as soon as I figure out something more interesting.

---

## code I've borrowed:
Let's get this out of the way first: as I intend to use this with the I2CDriver made by Excamera Labs, the i2cdriver library is written by James Bowman and licensed under the 3-clause BSD license. That's what the LICENSE file applies to - and, as yet, not the code I've written. Haven't figured a license out for that yet. It's not in a neat little submodule because VSCodium refused to play ball, and I don't want to spend a second more than slightly necessary on this project.

---

## what this is and what it does?
You hook up an I2CDriver and a GBS8200. Firmware file goes in, I2C bit stream goes out. The MCU gets programmed. Or the other way around - dumps firmware too. Might work with other FT232 based adapters and GBS8220 as well. Who knows. 

---

## how it does?
First, you'll have to figure out how to compile. The provided CMake project ran on my machine (Debian), YMMV.
Then, for reading, run mtv230dump. Params:
* -F - select firmware filename. Defaults to mtv230m_fw.bin.
* -D selects the FT232 device. Defaults to `/dev/ttyUSB0`.
* -O reads OSD memory instead of code
* -S enables silent mode
* -P sets read start page, defaults to 0
* -L sets read length in pages, defaults to whole memory (256 code, 72 OSD)

The mtv230flash tool has not been tested yet, so figure it out yourself.
The OSD decoder hasn't even been properly started on. 

---

## warnings (not the compiler kind)
- contains language not appropriate for children, or the HR department
- all testing done on "works on my machine" basis
- id est, treat the software as untested. Any results of you running it, observing it, or otherwise interacting with it are your own responsibility - including, but not limited to filesystem corruption, damage to hardware, mental health, physical health, reputation or karma, incurred wrath of god, etc.
- not really supposed to run on Windows quite yet
---

## build instructions
beats me. I used VSCodium with CMakeTools addon

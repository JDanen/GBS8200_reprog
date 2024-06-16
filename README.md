# GBS8200-reprog

or a tool for reprogramming the MTV230M microcontroller, found on the GBS8200 scaler board
---

## code I've borrowed:
Let's get this out of the way first: as I intend to use this with the I2CDriver made by Excamera Labs, the i2cdriver library is written by James Bowman and licensed under the 3-clause BSD license. That's what the LICENSE file applies to - and, as yet, not the code I've written. Haven't figured a license out for that yet. It's not in a neat little submodule because VSCodium refused to play ball, and I don't want to spend a second more than slightly necessary on this project.
---

## what this is and what it does?
You hook up an I2CDriver and a GBS8200. Firmware file goes in, I2C bit stream goes out. The MCU gets programmed. Or the other way around - dumps firmware too. Might work with other FT232 based adapters and GBS8220 as well. Who knows. 
---

## how it does?
First, you'll have to figure out how to compile. My setup was a bit too cursed to let it go public yet. Then, run as follows:
`<name of program> -F firmwarefile.bin -D /dev/ttyUSBn -[R|W]`
F selects firmare file to either flash onto MTV230M, or dump the firmware into. Defaults to `mtv230m_fw.bin`
D selects the FT232 device. Defaults to `/dev/ttyUSB0`
R or W is, respectively read or write mode.
---

## warnings (not the compiler kind)
- contains language not appropriate for children, or the HR department
- all testing done on "works on my machine" basis
- id est, treat the software as untested. Any results of you running it, observing it, or otherwise interacting with it are your own responsibility - including, but not limited to filesystem corruption, damage to hardware, mental health, physical health, reputation or karma, incurred wrath of god, etc.
- not really supposed to run on Windows quite yet
---

## build instructions
beats me. I used VSCodium with the C/C++ Runner addon.
---

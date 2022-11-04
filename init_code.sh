#! /bin/bash

# compile the source files
# note 'harpsicord' isn't a spelling mistake
gcc synthesizer/piano_voice.c -o piano -lasound -lpthread 
gcc synthesizer/harpsicord_voice.c -o harpsichord -lasound -lpthread
gcc synthesizer/flute_voice.c -o flute -lasound -lpthread
gcc synthesizer/guitar_voice.c -o guitar -lasound -lpthread 
gcc synthesizer/clarinet_voice.c -o clarinet -lasound -lpthread

# setup the config files for running executables
echo "1" > config
echo "50" > volume
echo "..." > proc_msg

# setup the bluetooth connection
# Check if the device is already connected and if not run these 2 commands
sudo sdptool add --channel=15 SP
rfcomm listen rfcomm0 15

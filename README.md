# AmiGUS-SW
Sample code and test cases for AmiGUS Sound card

## wavplay

Commandline WAV file reader and player for the AmiGUS. Two variants
are provided. 

The first variant called "aguswav" uses the main output sub-device (in 16 or 24 bit).
The latter mode is enabled by the "-24" command line option.

Secondly, "aguswtab" is another approach that utilizes the Wavetable engine and it's
interrupts for streaming playback. As of this writing, only 16 bit output is supported.




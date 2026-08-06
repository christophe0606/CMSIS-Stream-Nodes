# README

## FVP

The FVP simulation is __very__ slow. You have to wait for a few minutes before you can see a recognized keyword printed on stdout.

At the end of the simulation, an error will occur that is normal:
* The simulation reached the end of the audio file
* An underflow error is generated because the VSI driver is no more generating audio samples
* By default, the simulation is generating a panic when an error is occuring in the stream processing (it can be changed)

The recognition is having difficulties to distinguish `no` and `go`.
This will require some tuning in the audio processing.

Current simulation is generating (Zephyr):

```
[00:00:01.680,000] <inf> cmsisstream: KWS Classify: yes

[00:00:03.160,000] <inf> cmsisstream: KWS Classify: yes

[00:00:04.240,000] <inf> cmsisstream: KWS Classify: go

[00:00:05.240,000] <inf> cmsisstream: KWS Classify: yes

[00:00:06.400,000] <inf> cmsisstream: KWS Classify: go

[00:00:06.460,000] <inf> cmsisstream: KWS Classify: no

[00:00:07.400,000] <inf> cmsisstream: KWS Classify: yes

[00:00:08.700,000] <inf> cmsisstream: KWS Classify: go

[00:00:09.880,000] <inf> cmsisstream: KWS Classify: go

[00:00:10.400,000] <inf> cmsisstream: KWS Classify: no

[00:00:10.420,000] <inf> cmsisstream: KWS Classify: go

[00:00:10.440,000] <inf> cmsisstream: KWS Classify: no

[00:00:10.740,000] <inf> cmsisstream: KWS Classify: yes

[00:00:11.399,000] <dbg> cmsisstream: stopAudio: Stopping microphone source
```

## On board

The recognition is not working very weel.
The dataprocessing pipeline has been checked with the `debug` graph and it works.
THe problem is likely to be due to the sound from the microphone. It may need some tuning : gain etc ...

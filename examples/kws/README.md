# README

## FVP

The FVP simulation is __very__ slow. You have to wait for a few minutes before you can see a recognized keyword printed on stdout.

At the end of the simulation, an expected error occurs:

* The simulation reached the end of the audio file
* An underflow error is generated because the VSI driver no longer produces audio samples
* By default, a stream-processing error generates a panic; this policy can be changed

Recognition has difficulty distinguishing `no` and `go`.
This will require some tuning in the audio processing.

The current Zephyr simulation produces:

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

Recognition does not yet work reliably on the board.
The data-processing pipeline has been checked with the `debug` graph and works correctly.
The likely cause is the microphone input, which may require gain or other audio-front-end tuning.

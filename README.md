[README.md](https://github.com/user-attachments/files/30199366/README.md)
Graphic Equaliser (JUCE / C++)

A real-time three-band graphic equaliser plugin built in JUCE, developed for the Music and Audio Programming module (ECS7012P) at Queen Mary University of London.

Overview

The plugin gives independent gain control over low, mid, and high frequency bands using a serial chain of IIR filters:

- Low shelving filter** — boosts/cuts frequencies below ~250 Hz
- Peaking filter** — boosts/cuts around ~1000 Hz
- High shelving filter** — boosts/cuts frequencies above ~4000 Hz

Gain values are set via rotary sliders on the UI, bound to the DSP through `AudioProcessorValueTreeState`. Values are read in decibels and converted to linear scale before being applied per audio channel, so stereo signals are processed correctly.

The interface also includes basic playback controls (load, play, stop), so the EQ can be run on a loaded `.wav`/`.mp3` file, or on generated white noise if no file is loaded.

## Validation

The system was tested using white noise as input, since its flat spectral distribution makes gain-induced changes easy to observe. +10dB boosts were applied to individual bands and the output was recorded in Audacity, then compared against the unprocessed signal using spectral plots.

Results showed each boost was isolated to its intended frequency region with minimal bleed into neighbouring bands, confirming correct filter behaviour.

See [`Report.pdf`](./Report.pdf) for the full writeup, including spectral analysis and discussion.

## Project structure

```
GraphicEqualiser/
├── GraphicEqualiser.jucer   # JUCE project file (open in Projucer)
├── Source/
│   ├── PluginProcessor.h/.cpp   # DSP: filter chain, parameter handling, file playback
│   └── PluginEditor.h/.cpp      # UI: rotary sliders, buttons, layout
└── Report.pdf                # Full assignment report with spectral analysis
```

## Building

1. Open `GraphicEqualiser.jucer` in [Projucer](https://juce.com/get-juce/) (part of the JUCE framework).
2. Export to your IDE of choice (Xcode, Visual Studio, etc.) and build.
3. Requires the [JUCE](https://juce.com/) framework.

Author

Sanjay Yamasandhi Sundresh — MSc Advanced Electrical and Electronics Engineering, QMUL

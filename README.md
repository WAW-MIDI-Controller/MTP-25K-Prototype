[tooltip]: ## "Hi, I'm a tooltip!\nI'll fill you in on the details as we go!"



# WAW MIDI MTP-25K
A Camosun College Electronics Capstone Project
Dec, 2025

So, you've picked up a new MTP-25K MIDI controller. Thank you and congratulations!

This manual will run through an example where we:
  - connect an MTP-25K to a computer and a synthesizer;
  - play some notes using the different modes and features;
  - experiment with different modulation options;
  - and hack the firmware to customize our controller even further!

## Table of Contents
  - [Connecting to MIDI Devices](#connecting-to-midi-devices)
    - [Computer](#computer)
    - [Synthesizer](#sythesizer)
  - [Notes and Modes](#notes-and-modes)
    - [Touch Keys](#1-touch-keys)
      - [Octaves](#2-octaves)
    - [Play Modes](#play-modes)
      - [STANDARD](#standard)
      - [ARP](#3-arp)
      - [HOLD](#4-hold)
      - [ARP_HOLD](#5-arp_hold)
      - [Quick Reference](#play-mode-quick-reference)
    - [Customization](#customization)
      - [SHIFT](#shift)
        - [SHIFT + STANDARD](#6-shift--standard)
        - [SHIFT + ARP](#7-shift--arp)
        - [SHIFT + HOLD](#8-shift--hold)
        - [SHIFT + ARP_HOLD](#9-shift--arp_hold)
        - [Quick Reference](#customization-quick-reference)
  - [Modulation](#modulation)
    - [Pitch Bend Slider](#10-pitch-bend)
    - [Modulation Slider](#11-modulation-slider)
    - [XY Modulator](#12-xy-modulator)
  - [Firmware](#firmware)
  - [Glossary](#glossary)
  


In this example we'll use MIDI-OX, an open source [MIDI Utility Program](#midi-utility-program) that displays MIDI data and plays tones. Follow our [MIDI-OX instructions](Docs/MIDI-OX-Instructions.md) to set it up on your computer, or follow along in your favourite [Digital Audio Workstation](#digital-audio-workstation)!

# Connecting to MIDI Devices

The MTP-25K is equipped with a microUSB port to connect to a computer, and a MIDI-TRS jack to connect to other devices like synthesizers and drum machines.

## Computer

To connect to a computer, ensure that you are using a data-capable microUSB cable*. Once connected, the MTP-25K will appear as an available device in your [Digital Audio Workstation](#digital-aduio-workstation), and it is ready to play!

We will use this setup with MIDI-OX for our tutorial. Click (here) for instructions on connecting the MTP-25K to MIDI-OX.

<sub>*\*Some microUSB cables only transmit power. If the MTP-25K does not appear as an available device, this is likely the cause.*</sub>

## Synthesizer

When connecting to a synthesizer or drum machine, electronic musical instruments that create sound by generating and manipulating electrocial signals, use a microUSB cable to power the controller. This can be plugged into a computer or a USB to AC adapter.

If the synthesizer has a TRS MIDI input, simply run a TRS cable from the jack on the MTP-25K to the MIDI input.

If the synthesizer has a DIN MIDI input, plug a TRS to DIN adapter into the MTP-25K, and run a DIN cable from that to the MIDI input.

# Notes and Modes

In this section, we'll discuss the most important aspect of the MTP-25K: its ability to generate MIDI note signals! We'll start by playing the controller with its default startup settings. As we go, we'll adjust the octave range and musical features. Finally, we'll look at customizing the musical features.

## 1. Touch Keys

The 25 touch sensors along the bottom of the MTP-25K act as its keybed. The controller's default note range is C3 through C5. This centres us around C4, the middle C of a standard piano, lining up with the octave indicator. 

1. Touch and hold the middle key of the controller.
> An LED indicator *should* turn on above the key. You *should* hear a note and see "C" appear under DATA1 and "4" under DATA2 on the MIDI_OX Input Monitor. *Hopefully*, you played a C4!

2. Release the key.
> The note *should* stop, with a second event appearing on the Input Monitor.

3. Touch more keys.
> Music!

<sub>*If the LED indicators are not turning on, there is likely a power issue. Try another microUSB cable.*</sub>

<sub>*If the LED indicators turn on but the notes don't play or appear on MIDI-OX, there is likely a data transfer issue. Try another microUSB cable.*</sub>

<sub>*If the LED indicators turn on, the notes don't play, but they do appear on MIDI-OX, there is likely a MIDI-OX Port Routing issue. See MIDI_OX Port Routing.*</sub>

<sub>*Are your speakers/headphones muted?*</sub>

### 2. Octaves

The Octave Down and Up buttons move the MTP-25K's note range up and down, giving it an effective range of C1 to C8.

1. Press the Octave Down button.
> The octave indicator switches to C3. 

2. Touch the middle key again.
> A lower note plays and "3" appears under DATA2 on the Input Monitor. You played C3!

3. Press the Octave Down button again.
> The octave indicator bottoms out at C2.

4. Touch the far left key.
> You played C1, the lowest note!

5. Press the Octave Up button 5 times.
> The octave indicator hits its highest position, C7. 

6. Touch the far right key.
> You played C8, the highest note!

7. Press the Octave Down button 3 times.
> Back to C4, starting from scratch.

## Play Modes

Play modes are controlled with the ARP, or [arpeggio](#arpeggio), and HOLD, or [sustain](#sustain) buttons. These modes change the way that the controller sends MIDI Note On and MIDI Note Off commands.

### STANDARD

This is the default mode, the one you've been using so far. As with a piano, touching a key starts a note and releasing the key stops the note. 

*NOTE - Many digital instruments have a decay effect, where notes taper off after a few seconds to immitate acoustic instruments.*

### 3. ARP

1. Press the ARP button.
> The ARP LED indicator turns on.

2. Touch and hold a key. 
> The note plays periodically, in time with the flashing LED indicator<sup>*</sup>.

3. Release that key, then touch and hold two new keys.
> The two notes play back and forth at the same tempo.

4. Add two additional keys.
> The notes play from lowest to highest, looping back to the beginning each time.

5. Touch a fifth key.
> Rather than joining the loop, the note plays immediately.<sup>**</sup>

6. Try different combinations of up to four notes.
> More music!

7. Press the ARP button again.
> Back to STANDARD mode.

*Note - The ARP, HOLD, and SHIFT buttons can be toggled on and off with a short press, or held on with a long press.*

<sub>*\*This is the tempo indicator, and we discuss adjusting it in the Customize section.*</sub>

<sub>*\*\*This is due to the SET feature, which we discuss in the Customize section.*</sub>


### 4. HOLD

1. Press the HOLD button.
> The right HOLD LED indicator turns on.<sup>*</sup>

2. Touch and quickly release the far left key.
> The note continues to play until it decays. The key's LED indicator stays on.

3. Touch the key again.
> The key's LED indicator turns off.

4. On MIDI-OX, click the acoustic guitar icon to enter the Instrument Patch Panel.
> Try different instruments to find one that doesn't decay. Overdrive works.

5. Touch the far left key again to start the note, then press the Octave Down button.
> The LED indicator for the far left key will turn off and the LED indicator for the middle key will turn on.

6. Touch the middle key.
> The note stops, and the middle key's LED indicator turns off.

7. Press the Octave Up button once.
> Back to default note range.

8. Turn on and off different note combinations.
> Chords!

9. Press the HOLD button again.
> Notes stop playing. Back to STANDARD mode.

<sub>*\*There are two HOLD sub-modes, HOLD_ALL and HOLD_SET. This is HOLD_ALL, we discuss HOLD_SET in the Customize section*</sub>

### 5. ARP_HOLD

1. Press the ARP button, then the HOLD button, then the Octave Down button.
> The ARP and right HOLD LED indicators turn on, and the octave indicator switches to C3.

2. Touch and quickly release a key.
> The note plays periodically, in time with the tempo LED indicator.

3. Touch the key again.
> The note stops.

4. Touch four keys one after the other.
> The arpeggio forms as you add the notes.

5. Press the Octave Up button twice.
> The arpeggio continues, but the indicators do not turn on.

6. Touch more keys.
> Music with a baseline!

7. Press the Octave Down button.
> Back to default note range.

8. Press the HOLD button.
> The arpeggio continues. Still in ARP_HOLD mode.

9. Press the ARP button.
> The arpeggio stops. Back to STANDARD mode.

### Play Mode Quick Reference

See below for pseudo-code describing the behaviour of each mode.

#### ARP (arpeggio)
```
if press
  if arp buffer full
    start note
  else
    add note to buffer
    
else
  if note in arp buffer
    remove note from buffer
  else
    stop note
```

#### HOLD_ALL
```
if press
  if note is playing
    stop Note
  else
    start note

else
  //do nothing
```

#### HOLD_SET
```
if press
  if hold buffer full
    if note in hold buffer
      remove note from hold buffer
    else
      start note
  else
    add note to hold buffer

else
  if note in hold buffer
    //do nothing
  else
    stop note
```
#### ARP_HOLD
```
if press
  if arp buffer full
    if note in arp buffer
      remove note from arp buffer
    else
      start note
  else
    add note to arp buffer

else
  if note in arp buffer
    //do nothing
  else
    stop note
```  

So far we've covered playing notes in different octave ranges and different modes. Now, let's look at how we can customize those modes!

## Customization

Our gateway to customizations is the SHIFT button. With SHIFT enabled, the Octave Down and Octave Up buttons adopt different behaviours based on the mode status.

### 6. SHIFT + STANDARD

In STANDARD mode, with SHIFT enabled, the Octave Up and Down buttons adjust the controller's tempo.

1. Press the SHIFT button.
> The SHIFT LED indicator turns on.

2. Press the Octave Up button repeatedly.
> The tempo LED indicator flashes faster.

3. Press the Octave Down button repeatedly.
> The tempo LED indicator slows down.

### 7. SHIFT + ARP

In ARP mode, with SHIFT enabled, the Octave Up button cycles through the arpeggio mode (pattern) options and the Octave Down button cycles through the arpeggio note length.

1. Press the ARP button
> The ARP LED indicator turns on, the SHIFT LED indicator should still be on.

2. Press the Octave Up button.
> The ARP Mode LED indicator moves down by one.

3. Touch and hold four keys.
> The notes play from highest to lowest, looping back to the high note and repeating the pattern.

4. Release the keys.
> The arpeggio stops.

5. Press the Octave Down button.
> The ARP Note Length LED indicator moves down to Down.

6. Touch and hold four keys.
> The notes play at twice the speed, two notes per tempo flash.

7. Press the Octave Down button.
> The notes double in speed again, four notes per tempo flash.

8. Press the Octave Up button.
> The Arp Mode LED indicator moves down to Up/Down. The notes play from lowest to highest, then from highest to lowest, repeating the new pattern.

9. Release the keys.
> The arpeggio stops.

10. Press the Octave Up button.
> The Arp Mode LED indicator moves down to Custom.

11. Touch and hold four keys, but not in ascending or descending order.
> The note sequence plays in the order you pressed the notes.<sup>*</sup>

12. Press the ARP button.
> Back to STANDARD mode.

<sub>*\*If an arpeggio is started in Custom mode, the selected order will persist for other patterns. Similarly, if an arpeggio is started in any other modes, Custom will immitate Up mode.*<sub>

### 8. SHIFT + HOLD

In HOLD mode, with SHIFT enabled, the Octave Down button selects the HOLD_SET sub-mode and the Octave Up button selects the HOLD_ALL sub-mode.

1. Press the HOLD button.
> The right HOLD LED indicator turns on. The SHIFT LED indicator should still be on.

2. Press the Octave Down button.
> The right HOLD LED indicator turns off and the left HOLD LED indicator turns on.

3. Touch and release four different keys.
> The notes play continuously.

4. Touch and release a fifth key.
> The note plays immediately, and turns off when released.<sup>*</sup>

5. Press the HOLD button twice.
> The left HOLD LED indicator turns off and on. We're still in the HOLD_SET sub-mode.

6. Press the Octave Up button.
> The left HOLD LED indicator turns off and the right HOLD LED indicator turns on.

7. Touch and release five or more keys.
> All notes play continuously.

8. Press the HOLD button.
> The right HOLD LED indicator turns off, and all notes stop.

<sub>*\*This is again due to the SET feature, which we're about to cover!*</sub>

### 9. SHIFT + ARP_HOLD

In HOLD mode, with SHIFT enabled, the Octave Up and Octave Down buttons change the Set Number, adjusting how many notes are included in an arpeggio or held in the HOLD_SET sub-mode.

1. Press the ARP button and the HOLD button.
> The ARP and right HOLD LED indicators turn on. The SHIFT LED indicator should still be on.

2. Press the Octave Down button.
> The Set Number LED indicator moves to 3.

3. Touch and release three keys.
> The notes play in an arpeggio.

4. Touch and release a fourth key
> The note plays as in STANDARD mode.

5. Press the Octave Up button three times.
> The Set Number LED indicator moves to 8.

6. Touch and release more keys.
> The notes will add to the arpeggio until there are eight notes. After that, notes will play as if in STANDARD mode.

### Customization Quick Reference

See below for pseudo-code describing the behaviour of the octave keys when shift is enabled.

#### STANDARD
```
if octave up
  tempo up
    
if octave down
  tempo down
```

#### ARP
```
if octave up
  cycle arp pattern
    
if octave down
  cycle arp note length
```

#### HOLD_ALL
```
if octave up
  //do nothing
    
if octave down
  switch to HOLD_SET
```

#### HOLD_SET
```
if octave up
  switch to HOLD_ALL
    
if octave down
  //do nothing
```
#### ARP_HOLD
```
if octave up
  increase set number
    
if octave down
  decrease set number
```  

# Modulation

The modulation surfaces are still under construction, please check back in soon!

## 10. Pitch Bend Slider

Coming soon!

## 11. Modulation Slider

Coming soon!

# Firmware

Due to hardware problems, the current firmware is not at its most hackable. Please check in soon for firmware customization options!

# Glossary

### MIDI Utility Program

A program that shows MIDI data, but does not offer recording or audio manipulation.

### Digital Audio Workstation

A software that acts as a complete virtual recording studio, allowing users to record, edit, mix, and produce audio on a computer.

### Arpeggio

When arpeggiated, notes of a chord play in a sequence rather than simultaneously.

### Sustain

When sustained, notes continue to play after key is released.

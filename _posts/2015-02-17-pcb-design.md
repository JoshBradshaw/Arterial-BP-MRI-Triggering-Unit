---
layout: post
title: Designing the PCB
---

I opted to design a simple PCB for this project, so that more versions of this triggering unit can be produced with minimal effort. 


## Design Criteria

1. Analog ground plane must be completely separated from the digital groundplane to avoid crosstalk on the sensitive op-amp circuits.
2. Must use 10/10 thou minimum trace width and clearance, so that this board can be manufactured by anyone.
3. Must meet the weird drill size restrictions of [APcircuits](http://apcircuits.com/), because they've the only vendor I've who will ship a few boards to SickKids on a quick turnaround for a reasonable price.

I chose to use surface mount components with >1mm pin pitch for this design, because I wanted to be able to either manufacture these boards by hand, or have a manufacturer do them on a pick-and-place machine as required.

For anyone wishing to build this board, some moderate to advanced soldering is required. You can do it with very little experience, just follow this tutorial: [Sparkfun SMD soldering tutorial](https://www.sparkfun.com/tutorials/96). Make sure you buy top quality flux, solder and tweezers!

The Teensy eagle CAD library used in this design is available at the [PJRC website](https://www.pjrc.com/teensy/eagle_lib.html). 

## The board:

Design:

![alt text]({{ site.url }}pcb_layout.png "Circuit board layout")

Build Photos:

Assembled Board:



## Things to fix in subsequent revisions

1. One weird choice I made in the design process of this PCB was the decision to use a 6 channel digital potentiometer, when I'm currently only using two channels. I chose that part simply because I did the breadboard verion with a six channel pot, and because digikey didn't have the 2 or 4 channel version of the part in stock while I was rushing to get the prototype together in time for the first experiment.

## Related Questions and Uncertainty
1. How much electromagnetic inferferance from the MRI machie will the instruments be exposed to when they're sitting in the control room of the 3T scanner, assuming that we scan with the door closed and the waveguide plugged with a conductive blanket?
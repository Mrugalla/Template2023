/*
-----------------------------------------------------------------------------------------
~~~ todo ~~~

Oversampler
    make convolver FFT based
    implement 4x
xen
master tune
base note
pitchbend range
sidechain
    envfol on macro
MixProcessor
    code readability
    add latency compensation delay
    write cpp

-----------------------------------------------------------------------------------------
~~~ Block Diagrams: Signal Flow Layouts ~~~


----- Layout0: All -----

> [m/s enc] > [Layout1 Nonlinear/Linear] > [gainOut] > [m/s dec] >

----- Layout1: Nonlinear -----

> [split]                                               [mix/delta] >
     |                                                       |
  dry|> [ff delay] ------------------------------------------|
     |                                                       |
  wet|> [gainWetIn] > [Layout2] > [unityGain] > [gainWetOut]-|

----- Layout1: Linear -----

> [split]                          >
    |                              |
   dry|> [ff delay] > [gainDryOut]-|
    |                              |
   wet|> [Layout2] > [gainWetOut]--|

----- Layout2: Oversampling -----

> [upsampler] > [Layout3 Process 1x/2x/4x] > [downsampler] >

----- Layout3: Process -----

-----------------------------------------------------------------------------------------
~~~ unclear ~~~


*/
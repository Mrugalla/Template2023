/*
-----------------------------------------------------------------------------------------
~~~ todo ~~~

ToThink
    how to handle different components being lockable?
    how to handle different components being parameterized?
    should a button know its togglestate?

GUI
    Label
        can also draw images or paths (so buttons have easier time)
            setMaxHeight then doesn't do stuff
        Find or make best performing drawText method
            improve current one with suggestions from JUCE forum:
                https://forum.juce.com/t/drawing-text-without-clipping-label-bounds/58325/11
    Button
        cancel click on opposite mouse button
        OnPaint, Wheel
    TextEditor
        button that edits its label
    Editor
        makeToast (firstTimeUwU)
    Knob
        OnPaint, Click, Wheel, Drag
AUDIO
    FFT
        replace by 3rd party lib for performance (kiss fft or pffft)
    Oversampler
        make convolver FFT-based
        implement 4x
    sidechain
        envfol on macro
    MixProcessor
        code readability
        add latency compensation delay
        write cpp

-----------------------------------------------------------------------------------------

*/
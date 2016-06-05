# eurodrumpf

eurorack semi-euclidean beat sequencer

constructed using an Arduino Nano + Synovatron DIY Prototyping kit

5 pots (A1..A5) control 3 drums

* 'BD' - a beat (typically  going to a bass drum) has a single control) which is 'density' on scale of 1-8 (how many beats per 16)
	...these beats are always on a 'down beat'

* 'MD' and 'HD' - other beats (typically a midrange tom/snare and a high hat) which have 'density' (0-16)  and 'offset'
	.. offset changes the way the notes are distributed throughout the 16 beats 
			
			

there are 5 jacks - 2 inputs (D2/D3) and 3 outputs (D4-D6)

* CLK (D2) - input: advances the sequence through steps 0-15. Note that when a beat is output, the pulse width is the same as the input
clock pulse width. i.e. on a rising CLK signal, any drum which is meant to be played on the current step also goes high, and
when CLK goes low, all drums also go low
* RST (D3) - input : a rising edge forces the sequence to go to step 0

* BD - output: sequence with density controlled by the BD knob
* MD - output: sequence with density & offset controlled by the MD knobs
* HD - output: sequence with density & offset controlled by the HD knobs

 

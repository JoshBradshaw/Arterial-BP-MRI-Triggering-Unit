# Testing Against Fetal Lamb BP Data

Australian researcher Jonathan Mynard was kind enough to provide me with three ~30 second invasive pressure catheter recordings taken in a fetal lamb. These recordings are slightly unusual, because they have extra systolic beats, which lead to little skips in the waveform. These recordings are a useful analog for the fetal pig pressure waveform, because pigs are also prone to extra systolic beats, and fetal lambs and fetal pigs have similar fetal heart rates.

To test these values, I used an Arduino Due as a waveform generator, and generated a 1-3V signal, which I then divided down to 0.5-1V

Information about the heart rate of a sedated fetal pig in this paper: http://jap.physiology.org/content/90/4/1577 [Effects of morphine and naloxone on fetal heart rate and movement in the pig](http://jap.physiology.org/content/90/4/1577)

Test results:

Data File 1:

Here's a zoomed in snapshot of how the tr
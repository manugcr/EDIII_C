# TP FINAL

Sound sampler with UART communication for LPC1769 using drivers.

## Description.

For this script to work we first need to convert an audio file into hexadecimal/decimal values using software such as [HxD](https://mh-nexus.de/en/hxd/) or [Audacity](https://www.audacityteam.org/). After that, send these values through UART communication using Python. The LPC will receive them and play the audio repeatedly through a speaker. You can tweak the sound frequency in real-time using a potentiometer or select a different effect with buttons.

<p align="center">
  <img src="https://github.com/user-attachments/assets/1c9a9efe-53ce-4156-9cbf-5745ab26221d"><br>
  <em>Fig 1. Circuito</em>
</p>

## Information.
[Drivers for LPC1769](https://gitlab.com/GabrielEValenzuela/electronicadigital_iii)

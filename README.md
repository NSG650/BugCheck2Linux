# BugCheck2Linux
Windows crashed? Dropping you into a linux shell!



https://user-images.githubusercontent.com/51860844/236764849-0b6bc55e-f4ee-442f-83c7-7450ce74be49.mp4



# How to run it?

1. Enable test signing and reboot

```
bcdedit /set testsigning on
shutdown /r /t 0
```

2. Create a service using SC

```
sc create BugCheck2Linux binPath=C:\where\ever\the\driver\is\BugCheck2Linux.sys type=kernel start=demand
```

3. Run it!

```
sc start BugCheck2Linux
```

*4. In case you change your mind then unload the driver to revert the changes
```
sc stop BugCheck2Linux
```

# How does this work?
We simply register a BugCheck callback. The callback function runs a [tiny RISC V emulator](https://github.com/cnlohr/mini-rv32ima) running linux. 

For the video output we use bootvid.dll and for input we have a ~~horrible~~ simple polling based PS/2 keyboard driver.

# Limitations?

* No ANSI escape sequences supported.
* Runs at 640x480 with 16 colors.
* Only works on BIOS systems.
* Is really slow at times.

# Credits

[mini-rv32ima](https://github.com/cnlohr/mini-rv32ima) for the RISC V emulator and the Linux image.

[ReactOS Project](doxygen.reactos.org/) for providing documentations for bootvid

[OSdev Wiki](https://wiki.osdev.org/%228042%22_PS/2_Controller) for providing documentations for PS/2

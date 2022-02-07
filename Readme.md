## Bsnes v086

The *Bsnes v086* has an integrated debugger that is amazing. This debugger was implemented by Byuu/Near [https://near.sh/](https://near.sh/).

This repository contains some new features, listed below:

- More breakpoints
- The Disassembly window has more lines
- It has a new window for graphics features
- Now when the debugger is out of focus and you click on any window, all windows go to the front.

## Compiling the debugger

To compile this debugger you need a ***gcc*** version from the family *4.8.x*. Here I use version *4.8.5*.

Download or clone the repository and enter the *bsnes* folder and run the command:

```
make ui=ui-debugger
```

The binary is located in the *out* folder.


## Releases

Check the *Releases* section for the newest compiled binaries.


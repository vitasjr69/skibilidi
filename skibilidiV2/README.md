# skiBILidi

## Building and uploading the code to an Arduino
Use `make upload` to compile main.c and flash it onto the Arduino. Change the
`PORT` variable in the `Makefile` to the correct port for your Arduino. When that has succeded use
`make clean` to remove the unneeded files.

## Dependencies
To be able to use the `Makefile` you have to install some packages, on linux you would do this as
follows:

`sudo apt install avr-gcc avr-libc avrdude`

or on MacOS you can use homebrew:

`brew install avr-gcc avr-libc avrdude`

### Using flakes
If you are on NixOS using flakes you can use `nix develop` to enter the development environment
which allows you to use the `make` commands listed above.
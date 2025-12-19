# Place Where I Can Tweak STM32

Using C++ and CMake to make it all work.

## Build

### Configure:
```bash
cmake --preset debug
```

### Build:
```bash
cmake --build --preset debug --target blinky
```

### Flash:
```bash
cmake --build --preset debug --target flash
```

## USART Communication

To read output from the board you need this package:
```bash
sudo pacman -S uucp
```

And then run a command like this:
```bash
cu -l /dev/ttyACM0 -s 115200 
```

You might need to use another tty, check which one with:
```bash
ls -lt /dev/tty*
```

## Configuration VSCode

In .vscode you will find some sample configs that you can use to quickly start building and debugging the project from the VSCode itself.

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

## Configuration VSCode

In .vscode you will find some sample configs that you can use to quickly start building and debugging the project from the VSCode itself.

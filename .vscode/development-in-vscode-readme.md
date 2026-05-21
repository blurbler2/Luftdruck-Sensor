## Development on VS Code

This repository was developed on macOS, so the paths and flash tooling in the workspace currently assume a Mac-based setup.

### What you need installed

- Visual Studio Code
- STM32CubeMX, which generates the code after you change the hardware configuration
- GNU Arm Embedded toolchain for STM32
- `st-flash` for programming the board over ST-LINK
- CMake and Ninja
- An ST-LINK compatible board connected over USB

### Install and verify

If you are setting up a new Mac, this is the easiest starting point:

```sh
brew install cmake ninja stlink
```

Then install STM32CubeMX and the GNU Arm Embedded toolchain for STM32 using the ST installer or bundle you already use on your machine.

Optional checks if you want to confirm everything is on the PATH:

```sh
code --version
arm-none-eabi-gcc --version
st-flash --version
cmake --version
ninja --version
```

For flashing, connect the ST-LINK board by USB and make sure it is powered.

### Recommended VS Code extensions

- C/C++
- CMake Tools
- STM32Cube support, if you use the ST tooling outside the terminal

### macOS setup notes

- Homebrew is the easiest way to install CMake, Ninja, and `stlink`.
- The workspace currently uses the STM32Cube toolchain bundle installed under the user profile.
- If you clone this repo on another Mac, check whether the toolchain path in `/.vscode/tasks.json` matches your machine.
- If you clone it on a different computer, update the hard-coded tool paths or add them to your `PATH`.

### Build

- Use the Debug preset from CMake.
- The generated build output is expected in `build/Debug`.
- The workspace already points IntelliSense at `build/Debug/compile_commands.json`.

### Flash

- The VS Code task `Build and Flash STM32 (Debug)` builds the firmware, converts the ELF to a binary, then flashes it with ST-LINK.
- The task uses a connect-under-reset flash command so it can recover more reliably when the board is already running code.

### What this repo assumes

- The STM32Cube-generated source tree is part of the project.
- The custom 7-segment display helper in `Core/Src/SiebenSeg.c` must be compiled into the target.
- The board is programmed through ST-LINK, not through a bootloader workflow.

### If you continue work on another machine

1. Install the tools listed above.
2. Open the workspace in VS Code.
3. Reconfigure CMake if needed so the compiler and `st-flash` paths resolve.
4. Run the build-and-flash task once to confirm the board is reachable.

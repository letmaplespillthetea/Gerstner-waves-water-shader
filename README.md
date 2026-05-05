# Water Surface Simulation

An interactive 3D water surface simulation built with C++ and OpenGL. This project simulates realistic water waves using Gerstner wave mathematics and features an advanced post-processing pipeline with progressive bloom. The simulation includes a real-time UI powered by ImGui for tweaking parameters on the fly.

<img width="800" height="470" alt="image" src="https://s4.ezgif.com/tmp/ezgif-4176c072770cf1c1.gif" />
## Features

- **Gerstner Wave Simulation**: Mathematically procedural waves that you can stack, add, and tweak in real-time.
- **Progressive Bloom**: A high-quality downsample/upsample bloom post-processing pipeline (inspired by Catlike Coding).
- **Interactive UI (ImGui)**: Modify wave amplitudes, frequencies, directions, water colors, lighting, and post-processing thresholds instantly.
- **Free-fly Camera**: Navigate above and below the water surface. Entering the water dynamically shifts the environmental color to simulate depth.
- **Wireframe Mode**: View the underlying polygonal grid structure of the water mesh at the press of a button.

## Prerequisites

- **CMake** (v3.14 or higher)
- A **C++17** compatible compiler (GCC, Clang, MSVC, MinGW, etc.)
- **Git** (Required for CMake `FetchContent` to download dependencies automatically)

*Note: The project uses CMake `FetchContent` to automatically download and link GLFW, GLM, and ImGui. GLAD is included in the source tree.*

## Building the Project

1. Open your terminal/command prompt and navigate to the project directory:
   ```bash
   cd path/to/project
   ```

2. Generate the build files using CMake:
   ```bash
   cmake -B build
   ```

3. Compile the project:
   ```bash
   cmake --build build
   ```

The `shaders` folder will be automatically copied to the executable's output directory during the build process.

## Running the Simulation

After building, you can run the generated executable from the root directory:

**On Windows:**
```bash
.\build\WaterSimulation.exe
```

**On Linux/macOS:**
```bash
./build/WaterSimulation
```

## How to Use

Once the application is running, you can interact with the environment and the settings panel.

### Controls

- **Movement:** `W`, `A`, `S`, `D` to move forward, left, backward, and right.
- **Vertical Movement:** `Space` to move up, `Left Ctrl` to move down.
- **Look Around:** Hold `Right Mouse Button` and move your mouse to rotate the camera.
- **Zoom:** `Mouse Scroll` wheel to zoom in and out.
- **Wireframe Mode:** Press `F` to toggle wireframe rendering.
- **Manage Waves:** Press `+` to add a new wave, and `-` to remove the last added wave.
- **Exit:** Press `Esc` to close the application.

### The Settings Panel (ImGui)

On the left side of the screen, you will find an extensive control panel where you can modify the environment dynamically:

- **Global Settings:** Toggle V-Sync and Wireframe mode.
- **Appearance:** Adjust the RGB colors for the *Shallow Water* and *Deep Water* to get different oceanic aesthetics.
- **Lighting:** Change the sun's color, ambient light intensity, and tweak the directional light vector to simulate different times of day.
- **Post-Processing:** Control the *Bloom Intensity*, *Bloom Threshold* (what brightness gets blooming), and *Bloom Soft Knee* to soften the bloom effect transitions.
- **Waves:** Manage the Gerstner waves. You can expand the list to view all active waves and individually adjust their `Direction`, `Amplitude`, `Frequency`, `Speed`, and `Steepness`. You can also add or delete specific waves to see how they interact.

# OpenGL D-Day in Normandy

## Graphic Processing Project

**Student:** Tofan George  
**Group:** 30435  

## Table of Contents
1. [Project Overview](#project-overview)
2. [Scenario](#scenario)
   - [Scene and Objects Description](#scene-and-objects-description)
   - [Functionalities](#functionalities)
3. [Implementation Details](#implementation-details)
   - [Camera and Animation](#camera-and-animation)
   - [Shadows](#shadows)
   - [Visualization Modes](#visualization-modes)
   - [Cannon Animation System](#cannon-animation-system)
   - [Fog System](#fog-system)
   - [Cannon Shot System](#cannon-shot-system)
   - [Day-Night System](#day-night-system)
4. [User Manual](#user-manual)
---

## Project Overview
This project presents an OpenGL 3D scene depicting the arrival of Allied forces on the beaches of Normandy during WWII. The scene features realistic terrain, military vehicles, dynamic lighting, shadows, and various interactive elements.

## Scenario
### Scene and Objects Description
The scene includes:
- Submarines targeting Nazi cannons on the beach.
- Large boats transporting tanks towards the shore.
- Soldier transport boats navigating large waves.
- Nazi artillery and tanks defending the beach.
- Natural elements such as hills, waves, and sparse vegetation.
- Realistic texturing for vehicles and terrain.

### Functionalities
- **Camera Control & Automated Tour**: Free movement using `WASD` keys and mouse, or an automated battlefield tour (`T`).
- **Shadows**: Shadow mapping for dynamic shadows that change with lighting (`J/L`).
- **Surface Visualization Modes**: Switch between solid, wireframe, and point mode (`1/2/3`).
- **Animated Cannon**: Constantly rotating cannon searching for targets.
- **Fog System**: Togglable atmospheric fog (`F`).
- **Cannon Shooting**: Fires a missile with dynamic lighting effects (`X`).
- **Night Mode**: Changes skybox and lighting to a dark purple hue (`N`).
- **Fragment Discarding**: Alpha-based transparency rendering.
- **Thunder Sound Effect**: Plays a thunder sound effect (`V`).

## Implementation Details
### Camera and Animation
The `gps::Camera` class controls movement and perspective using `cameraPosition`, `cameraFrontDirection`, and `cameraUpDirection`. An automated tour interpolates between key waypoints to showcase the battlefield.

### Shadows
Uses shadow mapping:
1. **Depth Map Creation**: Renders scene from the light’s perspective, storing object depth.
2. **Shadow Evaluation**: Compares fragment depth to determine visibility.
   - Optimizations include bias adjustments to reduce shadow artifacts and performance tuning for efficient rendering.

### Visualization Modes
Switch between three rendering modes:
- **Solid Mode** (`1`): Fully textured and lit models.
- **Wireframe Mode** (`2`): Displays object edges.
- **Point-Based Mode** (`3`): Renders only object vertices.

### Cannon Animation System
A continuously rotating cannon animates smoothly using transformation matrices. Rotation speed is fine-tuned for natural movement, updating lighting and shadows dynamically.

### Fog System
Implemented via fragment shader:
- Calculates fog density based on distance.
- Linearly interpolates between object color and fog color.
- Optimized for performance while maintaining visual quality.

### Cannon Shot System
A point light effect simulates the flash of a cannon shot:
- Uses linear and quadratic attenuation factors.
- Gradual intensity fade to mimic realistic illumination.

### Day-Night System
Toggles between day and night by:
- Changing the skybox.
- Adjusting light intensity and color (`0.4f, 0.2f, 0.4f`).
- Creates a moody atmosphere for nighttime combat.

## User Manual
### Keyboard Controls:
- `W/A/S/D`: Move the camera.
- `1/2/3`: Switch between rendering modes.
- `F`: Toggle fog effect.
- `N`: Toggle night mode.
- `X`: Trigger cannon shot animation.
- `V`: Play thunder sound.
- `M`: Display depth map.
- `T`: Start automated camera tour.
- `J/L`: Adjust lighting position.
- `ESC`: Exit application.

### Mouse Controls:
- Move mouse to adjust camera direction.

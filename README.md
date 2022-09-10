# shaderbg

This program lets you render shaders as a wall paper.[0] It works on Wayland compositors that support wlr-layer-shell.


[0] This program was inspired by, has a similar command line interface as, [glpaper](https://hg.sr.ht/~scoopta/glpaper).

# Usage

```
shaderbg [-h|--fps F|--layer l] output-name shader.frag
```
The parameter `layer` should be one of 'background', 'bottom', 'top', 'overlay'.

shaderbg provides two uniforms to the fragment shader: `float time`, measured in
seconds since the program started; and `vec2 resolution`, the current frame size
in pixels.

`output-name` should be either the name of an output (on Sway, these can be determined using `swaymsg -t get_outputs`) or the value `*` to match any output. To prevent the shell from expanding the `*` symbol, write `shaderbg '*' shader.frag`.

A few example shaders are provided in the demo/ folder.

# Installation

Build with meson. Requires EGL, OpenGL, and wayland.

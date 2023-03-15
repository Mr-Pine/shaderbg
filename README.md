# shaderbg

This program lets you render shaders as a wall paper.[0] It works on Wayland compositors that support wlr-layer-shell.


[0] This program was inspired by, has a similar command line interface as, [glpaper](https://hg.sr.ht/~scoopta/glpaper).

# Usage

```
shaderbg [-h|--fps F|--layer l] output-name shader.frag
```
The parameter `layer` should be one of 'background', 'bottom', 'top', 'overlay'.

`output-name` should be either the name of an output (on Sway, these can be determined using `swaymsg -t get_outputs`) or the value `*` to match any output. To prevent the shell from expanding the `*` symbol, write `shaderbg '*' shader.frag`.


`shaderbg` runs shaders that conform roughly to the Shadertoy interface[0]. That is,
shaders should implement
```
void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
}
```

Currently supported uniforms are

* `float iTime` measured in
seconds since the program started
* `vec3 iResolution`, whose first two coordinates give the current frame size
in pixels.
* `float iTimeDelta`
* `float iFrame`
* `vec4 iMouse`


A few example shaders are provided in the demo/ folder.

[0] https://web.archive.org/web/20230301165944/https://www.shadertoy.com/howto

# Installation

Build with meson. Requires EGL, OpenGL, and wayland.

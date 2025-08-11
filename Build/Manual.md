# Milkwave Manual

This manual is a stub. User input and contributions are welcome! 

If you want to contribute or need help, it's a good idea to [open an issue](https://github.com/IkeC/Milkwave/issues) or [join the Milkwave Discord server](https://bit.ly/Ikes-Discord).

## Tab "Shader"

Converts GLSL shader code to HLSL shader code.

### Shadertoy examples

Some of the Shadertoy shaders that can be converted to a Milkwave preset using the Shader tab, along with the required manual corrections after conversion.

| Title | ID  | Corrections |
| ----- | --- | ------------------------------------- |
| Shader Art Coding Introduction | [mtyGWy](https://www.shadertoy.com/view/mtyGWy) | - |
| Cyber Fuji 2020 | [Wt33Wf](https://www.shadertoy.com/view/Wt33Wf) | - |
| Tunnel of Lights | [w3KGRK](https://www.shadertoy.com/view/w3KGRK) | - |
| String Theory 2 | [33sSzf](https://www.shadertoy.com/view/33sSzf) | - |
| Fractal Pyramid | [tsXBzS](https://www.shadertoy.com/view/tsXBzS) | replace `break;` with `i=64.;` |
| CineShader Lava | [3sySRK](https://www.shadertoy.com/view/3sySRK) | replace `break;` with `i=64;` / replace aspect correction with `uv.x *= aspect.x;` / remove flipping |

### Resources

- [An introduction to Shader Art Coding](https://www.youtube.com/watch?v=f4s1h2YETNY) (Video)
- [How to create Presets from OpenGL Shaders](https://www.youtube.com/watch?v=Ur2gPa996Aw) (Video)
- [GLSL-to-HLSL reference](https://learn.microsoft.com/en-us/windows/uwp/gaming/glsl-to-hlsl-reference)
- [MilkDrop Preset Authoring Guide](https://www.geisswerks.com/milkdrop/milkdrop_preset_authoring.html#3f)
- [Shadertoy How-To](https://www.shadertoy.com/howto)

### Known limitations

Here are some common terms that cannot be converted automatically and need to be edited manually after conversion.

| Term | Info |
| ---- | ---- |
| `break` | Replace with a statement setting a condition to end the loop |
| `radians(a)` | Multiply by π/180 directly: `a * (M_PI/180)` |
| `atan(a,b)` | `atan2(a,b)` |
| `float[3] arr` | `float arr[3]` |
| `int[3] arr = int[](1,2,3)` | `int arr[3] = {1,2,3}` |
| `int ix = i & 1` (bitwise) | `int ix = i % 2` |
| `int yx = y >> 1` (bitwise) | `int yx = y / 2` |


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
| Fractal Pyramid | [tsXBzS](https://www.shadertoy.com/view/tsXBzS) | replace `break;` with `i=64.;`|
| CineShader Lava | [3sySRK](https://www.shadertoy.com/view/3sySRK) | rename `float time` / replace `break;` / replace aspect correction with `uv.x *= aspect.x;` / remove flipping |

### Resources

- [An introduction to Shader Art Coding](https://www.youtube.com/watch?v=f4s1h2YETNY) (Video)
- [How to create Presets from OpenGL Shaders](https://www.youtube.com/watch?v=Ur2gPa996Aw) (Video)
- [GLSL-to-HLSL reference](https://learn.microsoft.com/en-us/windows/uwp/gaming/glsl-to-hlsl-reference)
- [MilkDrop Preset Authoring Guide](https://www.geisswerks.com/milkdrop/milkdrop_preset_authoring.html#3f)
### Known limitations

| Term | Info |
| ---- | ---- |
| `break` | Replace with a statement setting a condition to end the loop |
| `atan(a,b)` | Use `atan2(a,b)` instead |
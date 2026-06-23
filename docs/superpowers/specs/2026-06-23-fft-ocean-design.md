# Acerola-Style FFT Ocean — Design Spec

Date: 2026-06-23
Branch: `feature/fft-ocean`

## Goal

Add a real-time, FFT-based ocean simulation (Tessendorf / Acerola style) to the
bengine OpenGL engine: GPU compute generates animated displacement/normal/foam
maps from a JONSWAP spectrum across multiple cascades, rendered as a displaced
grid with physically-flavoured water shading and an analytic sky for reflections.

## Decisions (from brainstorming)

- **Approach:** true FFT ocean on **OpenGL 4.3** compute shaders (engine upgraded
  from 3.3). GLAD already exports the 4.3 entry points — no regeneration needed.
- **Features:** FFT displacement + normals, Jacobian foam, water shading
  (Fresnel + sky reflection + sun specular), and multi-cascade waves.
- **FFT size:** N = 256, LOG_N = 8.
- **Cascades:** 3, patch sizes L ≈ {256, 64, 16} metres (swell → ripples).
- **Sky:** Hosek-Wilkie requested. Implemented as an **analytic Preetham** sky —
  Hosek-Wilkie needs a large published radiance-coefficient dataset that cannot be
  reproduced faithfully by hand, and the brainstorm pre-approved Preetham as the
  fallback. Preetham is self-contained and visually close for a reflection sky.

## Engine conventions

All new types are **header-only** (matching `shader.h`, `camera.h`, `model.h`,
`my_texture.h`), so **no `.vcxproj` changes** are required — `#include` pulls them
in. Shaders load from disk at runtime via relative paths, like the existing ones.

## Components

### ComputeShader (`headers/compute_shader.h`)
Mirrors `Shader`: loads one `.comp`, compiles `GL_COMPUTE_SHADER`, links, exposes
`use()`, `dispatch(x,y,z)` (wraps `glDispatchCompute` + `glMemoryBarrier`), and the
same uniform setters plus `setVec2`. `Shader` gains `setVec2`/`setVec4`.

### Ocean (`headers/ocean.h`)
Owns, per cascade: H0 spectrum texture, two packed frequency textures, two
ping-pong IFFT buffers, displacement texture, normal+foam texture. Shares one
butterfly (twiddle/bit-reversal) texture across cascades. Owns a CPU-generated
subdivided grid mesh (GRID×GRID quads). `update(time)` runs the compute chain;
`draw(view, proj, cameraPos, sunDir, sky)` renders the displaced, shaded surface.

Compute passes (per frame unless noted):
1. `spectrum_init.comp` — JONSWAP × Gaussian → H0 (run once / on param change).
2. `spectrum_update.comp` — evolve H0 by dispersion ω(k); output packed complex
   fields: packed0 = (h̃ height | D̃x), packed1 = (D̃z | 0).
3. `butterfly_precompute.comp` — twiddles + bit-reversed indices (run once).
4. `ifft.comp` — generic complex IFFT; ping-pong 8 horizontal + 8 vertical stages,
   applied to each packed texture (butterfly acts independently on `.xy` and `.zw`).
5. `ocean_merge.comp` — sign-correct + normalize the IFFT result into the
   displacement map; derive normals and Jacobian foam by finite differences of
   neighbouring displacement texels.

### Sky (`headers/sky.h`)
Analytic Preetham model. CPU computes per-channel coefficients from turbidity +
sun direction at init/when the sun moves; uploaded as uniforms. `sky.vert/.frag`
render a background dome; the same GLSL radiance function is reused inside
`ocean.frag` to colour reflections.

### Rendering shaders
- `ocean.vert` — sums the 3 cascade displacement maps (each tiled by its L) and
  displaces the grid vertices; passes world pos + uv for the cascades.
- `ocean.frag` — Schlick Fresnel blends deep-water colour ↔ Preetham sky
  reflection; adds sun specular off the FFT normal; blends Jacobian foam; cheap
  wave-peak subsurface tint.

### Integration (`main/main.cpp`)
Bump context hint to 4.3. Instantiate `Sky` and `Ocean`. In the render loop draw
the sky, then `ocean.update(time)` + `ocean.draw(...)`. Keep the existing scene
behind a runtime toggle (key) so prior work is preserved.

## Tunable parameters
Wind dir/speed, gravity, JONSWAP fetch, choppiness λ, foam threshold/decay,
cascade patch sizes L[3], displacement/normal strength, sun direction, turbidity,
deep/shallow water colours.

## Verification (limitation)
No msbuild/Visual Studio is available in the implementation environment, so the
author cannot compile or run this here. Mitigations: code strictly to GL 4.3,
explicit image-unit bindings per pass, `glGetError`/debug-output checks at init.
Final visual verification is performed by the user in Visual Studio.

## Out of scope (YAGNI)
Tessellation/LOD, screen-space reflections, buoyancy/physics, underwater fog,
GPU readback for CPU collision. Single flat grid, no LOD ring.

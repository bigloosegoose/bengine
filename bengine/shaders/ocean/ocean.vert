#version 430 core

// Displaces a flat world-space grid by the summed FFT displacement of all cascades.

layout(location = 0) in vec3 aPos;   // base grid position (world XZ, y = 0)

uniform mat4 view;
uniform mat4 projection;

uniform sampler2D displacement0;
uniform sampler2D displacement1;
uniform sampler2D displacement2;
uniform float L0;
uniform float L1;
uniform float L2;

out vec3 WorldPos;
out vec2 Uv0;
out vec2 Uv1;
out vec2 Uv2;

void main() {
	Uv0 = aPos.xz / L0;
	Uv1 = aPos.xz / L1;
	Uv2 = aPos.xz / L2;

	vec3 disp = texture(displacement0, Uv0).xyz
	          + texture(displacement1, Uv1).xyz
	          + texture(displacement2, Uv2).xyz;

	vec3 world = aPos + disp;
	WorldPos = world;
	gl_Position = projection * view * vec4(world, 1.0);
}

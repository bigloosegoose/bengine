#version 430 core

// Skybox-style sky dome. The cube position doubles as the view direction.

layout(location = 0) in vec3 aPos;

out vec3 vDir;

uniform mat4 view;        // view matrix with translation removed
uniform mat4 projection;

void main() {
	vDir = aPos;
	vec4 pos = projection * view * vec4(aPos, 1.0);
	gl_Position = pos.xyww;   // force depth = 1.0 so the sky sits behind everything
}

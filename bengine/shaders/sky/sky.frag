#version 430 core

// Analytic Preetham et al. (1999) sky model. (Hosek-Wilkie was requested, but its
// large published radiance dataset can't be reproduced faithfully by hand;
// Preetham is the pre-approved self-contained fallback and reads very close for a
// reflection sky.) The same preethamSky() function is duplicated in ocean.frag.

in  vec3 vDir;
out vec4 FragColor;

uniform vec3  sunDir;     // normalized, world space
uniform float turbidity;  // ~2 (clear) .. 10 (hazy)
uniform float exposure;

const float PI = 3.14159265358979;

vec3 perez(float cosTheta, float gamma, float cosGamma,
           vec3 A, vec3 B, vec3 C, vec3 D, vec3 E) {
	return (1.0 + A * exp(B / cosTheta)) *
	       (1.0 + C * exp(D * gamma) + E * cosGamma * cosGamma);
}

vec3 preethamSky(vec3 dir, vec3 sun, float T) {
	dir.y = max(dir.y, 0.001);
	sun.y = max(sun.y, 0.001);

	float thetaS   = acos(clamp(sun.y, 0.0, 1.0));
	float cosTheta = max(dir.y, 0.001);
	float cosGamma = clamp(dot(dir, sun), -1.0, 1.0);
	float gamma    = acos(cosGamma);

	vec3 A = vec3( 0.1787 * T - 1.4630, -0.0193 * T - 0.2592, -0.0167 * T - 0.2608);
	vec3 B = vec3(-0.3554 * T + 0.4275, -0.0665 * T + 0.0008, -0.0950 * T + 0.0092);
	vec3 C = vec3(-0.0227 * T + 5.3251, -0.0004 * T + 0.2125, -0.0079 * T + 0.2102);
	vec3 D = vec3( 0.1206 * T - 2.5771, -0.0641 * T - 0.8989, -0.0441 * T - 1.6537);
	vec3 E = vec3(-0.0670 * T + 0.3703, -0.0033 * T + 0.0452, -0.0109 * T + 0.0529);

	float chi = (4.0 / 9.0 - T / 120.0) * (PI - 2.0 * thetaS);
	float Yz  = (4.0453 * T - 4.9710) * tan(chi) - 0.2155 * T + 2.4192;

	float t  = thetaS;
	float t2 = t * t;
	float t3 = t2 * t;
	float T2 = T * T;
	float xz = ( 0.00166 * t3 - 0.00375 * t2 + 0.00209 * t) * T2
	         + (-0.02903 * t3 + 0.06377 * t2 - 0.03202 * t + 0.00394) * T
	         + ( 0.11693 * t3 - 0.21196 * t2 + 0.06052 * t + 0.25886);
	float yz = ( 0.00275 * t3 - 0.00610 * t2 + 0.00317 * t) * T2
	         + (-0.04214 * t3 + 0.08970 * t2 - 0.04153 * t + 0.00516) * T
	         + ( 0.15346 * t3 - 0.26756 * t2 + 0.06670 * t + 0.26688);

	vec3 zenith = vec3(Yz, xz, yz);
	vec3 num = perez(cosTheta, gamma,  cosGamma, A, B, C, D, E);
	vec3 den = perez(1.0,      thetaS, sun.y,    A, B, C, D, E);
	vec3 Yxy = zenith * (num / den);

	float Yc = Yxy.x;
	float xc = Yxy.y;
	float yc = max(Yxy.z, 1e-4);
	float X = (xc / yc) * Yc;
	float Z = ((1.0 - xc - yc) / yc) * Yc;
	vec3 rgb = vec3( 3.2406 * X - 1.5372 * Yc - 0.4986 * Z,
	                -0.9689 * X + 1.8758 * Yc + 0.0415 * Z,
	                 0.0557 * X - 0.2040 * Yc + 1.0570 * Z);
	rgb = max(rgb, vec3(0.0)) * 0.05;   // scale down absolute luminance

	float sunDisk = smoothstep(0.9996, 0.99985, cosGamma);
	rgb += vec3(sunDisk);
	return rgb;
}

void main() {
	vec3 dir = normalize(vDir);
	vec3 col = preethamSky(dir, normalize(sunDir), turbidity);
	col = vec3(1.0) - exp(-exposure * col);   // simple tonemap
	FragColor = vec4(col, 1.0);
}

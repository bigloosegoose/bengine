#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "shader.h"
#include "compute_shader.h"

// Acerola-style FFT ocean. Each cascade runs a GPU pipeline:
//   spectrum_update -> inverse FFT (ping-pong) -> merge (displacement + normal/foam)
// The displaced grid is then shaded by ocean.frag with Preetham sky reflections.
class Ocean {
public:
	static const int N        = 256;   // FFT resolution (power of two)
	static const int LOG_N    = 8;     // log2(N)
	static const int CASCADES = 3;
	static const int GRID     = 256;   // surface mesh subdivisions

	// world span of the mesh; cascade 0 tiles once across it
	float worldSize = 256.0f;
	float L[CASCADES] = { 256.0f, 64.0f, 16.0f };  // cascade patch sizes

	// surface look (tunable)
	glm::vec3 deepColor    = glm::vec3(0.02f, 0.08f, 0.12f);
	glm::vec3 shallowColor = glm::vec3(0.10f, 0.30f, 0.36f);

	Ocean()
		: specInit(R"(shaders/ocean/spectrum_init.comp)"),
		  specUpdate(R"(shaders/ocean/spectrum_update.comp)"),
		  butterflyPre(R"(shaders/ocean/butterfly_precompute.comp)"),
		  ifft(R"(shaders/ocean/ifft.comp)"),
		  merge(R"(shaders/ocean/ocean_merge.comp)"),
		  oceanShader(R"(shaders/ocean/ocean.vert)", R"(shaders/ocean/ocean.frag)")
	{
		butterflyTex = makeTex(LOG_N, N);
		for (int c = 0; c < CASCADES; c++) {
			cas[c].h0          = makeTex(N, N);
			cas[c].packed0     = makeTex(N, N);
			cas[c].packed1     = makeTex(N, N);
			cas[c].pingpong0   = makeTex(N, N);
			cas[c].pingpong1   = makeTex(N, N);
			cas[c].displacement= makeTex(N, N, true);
			cas[c].normalFoam  = makeTex(N, N, true);
		}

		precomputeButterfly();
		initSpectra();
		buildGrid();
	}

	void update(float time) {
		for (int c = 0; c < CASCADES; c++) {
			// 1. time evolution -> frequency-domain packed fields
			specUpdate.use();
			specUpdate.setInt("N", N);
			specUpdate.setFloat("L", L[c]);
			specUpdate.setFloat("time", time);
			bindImage(0, cas[c].h0,      GL_READ_ONLY);
			bindImage(1, cas[c].packed0, GL_WRITE_ONLY);
			bindImage(2, cas[c].packed1, GL_WRITE_ONLY);
			specUpdate.dispatch(N / 16, N / 16, 1);

			// 2. inverse FFT each packed field (result lands back in packedX)
			runIFFT(cas[c].packed0, cas[c].pingpong0);
			runIFFT(cas[c].packed1, cas[c].pingpong1);

			// 3. assemble displacement + normal/foam
			merge.use();
			merge.setInt("N", N);
			merge.setFloat("L", L[c]);
			merge.setFloat("choppiness", choppiness);
			merge.setFloat("dispScale", dispScale);
			merge.setFloat("foamBias", foamBias);
			merge.setFloat("foamScale", foamScale);
			bindImage(0, cas[c].packed0,     GL_READ_ONLY);
			bindImage(1, cas[c].packed1,     GL_READ_ONLY);
			bindImage(2, cas[c].displacement, GL_WRITE_ONLY);
			bindImage(3, cas[c].normalFoam,   GL_WRITE_ONLY);
			merge.dispatch(N / 16, N / 16, 1);
		}
	}

	void draw(const glm::mat4& view, const glm::mat4& projection,
	          glm::vec3 cameraPos, glm::vec3 sunDir, float turbidity, float exposure) {
		oceanShader.use();
		oceanShader.setMat4("view", view);
		oceanShader.setMat4("projection", projection);
		oceanShader.setVec3("cameraPos", cameraPos);
		oceanShader.setVec3("sunDir", sunDir);
		oceanShader.setFloat("turbidity", turbidity);
		oceanShader.setFloat("exposure", exposure);
		oceanShader.setVec3("deepColor", deepColor);
		oceanShader.setVec3("shallowColor", shallowColor);
		oceanShader.setFloat("L0", L[0]);
		oceanShader.setFloat("L1", L[1]);
		oceanShader.setFloat("L2", L[2]);

		// displacement -> units 0,1,2 ; normal/foam -> units 3,4,5
		const char* dispNames[3]   = { "displacement0", "displacement1", "displacement2" };
		const char* normNames[3]   = { "normalFoam0",   "normalFoam1",   "normalFoam2" };
		for (int c = 0; c < CASCADES; c++) {
			glActiveTexture(GL_TEXTURE0 + c);
			glBindTexture(GL_TEXTURE_2D, cas[c].displacement);
			oceanShader.setInt(dispNames[c], c);
			glActiveTexture(GL_TEXTURE0 + 3 + c);
			glBindTexture(GL_TEXTURE_2D, cas[c].normalFoam);
			oceanShader.setInt(normNames[c], 3 + c);
		}

		glBindVertexArray(meshVAO);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
	}

	// simulation tunables
	float choppiness = 1.2f;
	float dispScale  = 1.0f;
	float foamBias   = 0.6f;
	float foamScale  = 1.5f;
	float windSpeed  = 32.0f;
	float amplitude  = 600.0f;   // see initSpectra(): primary wave-height knob
	glm::vec2 windDir = glm::normalize(glm::vec2(1.0f, 0.6f));

private:
	struct Cascade {
		unsigned int h0, packed0, packed1, pingpong0, pingpong1, displacement, normalFoam;
	};
	Cascade cas[CASCADES];
	unsigned int butterflyTex;

	ComputeShader specInit, specUpdate, butterflyPre, ifft, merge;
	Shader oceanShader;

	unsigned int meshVAO = 0, meshVBO = 0, meshEBO = 0;
	int indexCount = 0;

	unsigned int makeTex(int w, int h, bool sampleLinear = false) {
		unsigned int tex;
		glGenTextures(1, &tex);
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
		GLenum filt = sampleLinear ? GL_LINEAR : GL_NEAREST;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filt);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filt);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		return tex;
	}

	void bindImage(unsigned int unit, unsigned int tex, GLenum access) {
		glBindImageTexture(unit, tex, 0, GL_FALSE, 0, access, GL_RGBA32F);
	}

	void precomputeButterfly() {
		butterflyPre.use();
		butterflyPre.setInt("N", N);
		bindImage(0, butterflyTex, GL_WRITE_ONLY);
		butterflyPre.dispatch(LOG_N, N / 16, 1);
	}

	void initSpectra() {
		for (int c = 0; c < CASCADES; c++) {
			specInit.use();
			specInit.setInt("N", N);
			specInit.setFloat("L", L[c]);
			specInit.setVec2("windDir", windDir);
			specInit.setFloat("windSpeed", windSpeed);
			// PRIMARY TUNING KNOB: the 1/N^2 IFFT normalisation makes wave height very
			// sensitive to this. ~600 gives roughly metre-scale swells at L=256; lower
			// it for calmer water, raise it for storms. (Tune together with dispScale.)
			specInit.setFloat("amplitude", amplitude);
			specInit.setFloat("suppress", L[c] / float(N) * 2.0f);
			specInit.setUint("seed", 1234u + c * 977u);
			bindImage(0, cas[c].h0, GL_WRITE_ONLY);
			specInit.dispatch(N / 16, N / 16, 1);
		}
	}

	// inverse FFT of one packed (two-complex) texture; result ends in `a`
	void runIFFT(unsigned int a, unsigned int b) {
		ifft.use();
		bindImage(0, butterflyTex, GL_READ_ONLY);
		unsigned int src = a, dst = b;
		for (int dir = 0; dir < 2; dir++) {
			ifft.setInt("direction", dir);
			for (int stage = 0; stage < LOG_N; stage++) {
				ifft.setInt("stage", stage);
				bindImage(1, src, GL_READ_ONLY);
				bindImage(2, dst, GL_WRITE_ONLY);
				ifft.dispatch(N / 16, N / 16, 1);
				unsigned int tmp = src; src = dst; dst = tmp;
			}
		}
		// 16 stages = even number of swaps -> result is back in `a`
	}

	void buildGrid() {
		std::vector<float> verts;
		std::vector<unsigned int> indices;
		float half = worldSize * 0.5f;
		float step = worldSize / float(GRID);
		for (int j = 0; j <= GRID; j++) {
			for (int i = 0; i <= GRID; i++) {
				verts.push_back(-half + step * i); // x
				verts.push_back(0.0f);             // y
				verts.push_back(-half + step * j); // z
			}
		}
		int stride = GRID + 1;
		for (int j = 0; j < GRID; j++) {
			for (int i = 0; i < GRID; i++) {
				unsigned int a = j * stride + i;
				unsigned int b = a + 1;
				unsigned int c = a + stride;
				unsigned int d = c + 1;
				indices.push_back(a); indices.push_back(c); indices.push_back(b);
				indices.push_back(b); indices.push_back(c); indices.push_back(d);
			}
		}
		indexCount = (int)indices.size();

		glGenVertexArrays(1, &meshVAO);
		glGenBuffers(1, &meshVBO);
		glGenBuffers(1, &meshEBO);
		glBindVertexArray(meshVAO);
		glBindBuffer(GL_ARRAY_BUFFER, meshVBO);
		glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshEBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glBindVertexArray(0);
	}
};

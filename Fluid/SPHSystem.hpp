#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "Particle.hpp"
#include "GLTools.hpp"

#define PI 3.14159265f

struct SPHSettings {
	SPHSettings(float restDensity, float gasConst, float viscosity, float surfaceTension,
		float h, float g, float boundary, float rewindConstant, glm::vec3 target)
		: restDensity(restDensity)
		, gasConstant(gasConst)
		, viscosity(viscosity)
		, surfaceTension(surfaceTension)
		, h(h)
		, g(g)
		, boundary(boundary)
		, rewindConstant(rewindConstant)
		, target(target)
	{
		//mass = pow(2 / 3.f * h, 3) * restDensity;
		mass = 1;
		poly6 = 315.0f / (64.0f * PI * pow(h, 9));
		spikyGrad = -45.0f / (PI * pow(h, 6));
		spikyLap = 45.0f / (PI * pow(h, 6));
		h2 = h * h;
		selfDens = mass * poly6 * pow(h, 6);
		massPoly6Product = mass * poly6;
	}

	float poly6, spikyGrad, spikyLap, gasConstant, mass, h2, selfDens,
		restDensity, viscosity, h, g, boundary, massPoly6Product, surfaceTension, rewindConstant;
	glm::vec3 target;
};

struct SPHSystem {
private:
	SPHSettings settings;
	size_t particleCubeWidth;
	bool started;

	float CalculateKernel(float diff, float h) {
		float q = diff / h;
		float f;

		if (0 <= q && q < 1) f = 2 / 3.f - (q * q) + 0.5 * (q * q * q);
		else if (1 <= q && q < 2) f = 1 / 6.f * (2 - q) * (2 - q) * (2 - q);
		else f = 0;
		f *= 3 / (2 * PI);

		return 1 / (h * h * h) * f;
	}

	void boxBoundaryParticles() {
		float space = settings.h;
		int width = settings.boundary / space;
		for (int x = 0; x <= 2 * width; x++) {
			for (int y = 0; y <= 2 * width; y++) {
				for (int z = 0; z <= 2 * width; z++) {
					if (x * y * z == 0 || x == 2 * width || y == 2 * width || z == 2 * width) {
						Particle* particle = new Particle;
						particle->position = settings.h * glm::vec3(x, y, z);
						particle->isBoundary = true;
						particle->density = settings.restDensity;
						particles.push_back(particle);
					}
				}
			}
		}

		for (int piIndex = 0; piIndex < particles.size(); piIndex++) {
			Particle* pi = particles[piIndex];
			float pDensity = 0;

			for (auto pjIndex = 0; pjIndex < particles.size(); pjIndex++) {
				if (piIndex == pjIndex) continue;
				Particle* pj = particles[pjIndex];
				auto diff = length(pi->position - pj->position);
				pDensity += settings.mass * CalculateKernel(diff, settings.h);
			}
			pi->density = pDensity + settings.restDensity;

			float pPressure = settings.gasConstant * (pow(pi->density / settings.restDensity, 7) - 1);
			pi->pressure = pPressure;
		}
	}

	void initParticles() {
		std::srand(1024);
		for (int i = 0; i < particleCubeWidth; i++) {
			for (int j = 0; j < particleCubeWidth; j++) {
				for (int k = 0; k < particleCubeWidth; k++) {
					float ranX
						= (float(rand()) / float((RAND_MAX)) - 0.5f)
						* settings.h / 20;					 
					float ranY								 
						= (float(rand()) / float((RAND_MAX)) - 0.5f)
						* settings.h / 20;					 
					float ranZ								 
						= (float(rand()) / float((RAND_MAX)) - 0.5f)
						* settings.h / 20;




					//glm::vec3 nParticlePos = glm::vec3(
					//	settings.boundary - particleCubeWidth * settings.h / 2 + i * settings.h + ranX,
					//	settings.boundary - particleCubeWidth * settings.h / 2 + j * settings.h + ranY,
					//	settings.boundary - particleCubeWidth * settings.h / 2 + k * settings.h + ranZ);
					glm::vec3 nParticlePos = glm::vec3(
						i + ranX + 3,
						j + ranY,
						k + ranZ + 3);
					Particle* particle = new Particle;
					particle->position = nParticlePos;
					particles.push_back(particle);
				}
			}
		}
	}



public:
	SPHSystem(size_t particleCubeWidth, const SPHSettings& settings)
		: particleCubeWidth(particleCubeWidth), settings(settings) {
		//boxBoundaryParticles();
		initParticles();
		particleCount = particles.size();

	}

	std::vector<Particle*> particles;
	size_t particleCount;

	void draw() {
		for (int i = 0; i < particleCount; i++) {
			glm::vec3 p = particles[i]->position;
			//if(!particles[i]->isClose) 
			//	drawSphere(p, 0.1, glm::vec4(0.1, 0.8, 0.1, 1));
			//else
			//	drawSphere(p, 0.1, glm::vec4(0.1, 0.1, 0.8, 1));
			drawSphere(p, 0.1, glm::vec4(0.1, 0.8, 0.1, 1));
			//if (length(particles[i]->normal) > 0.001) {
			//	drawSphere(p, 0.1, glm::vec4(0.1, 0.8, 0.1, 1));
			//	drawCylinder(p, p + -100.f * particles[i]->normal, 0.03f);
			//}
			//if (particles[i]->isClose) drawSphere(p, 0.1, glm::vec4(1, 0, 0, 0));
			//else drawSphere(p, 0.1);
			//drawSphere(p, 0.1);
			//if (particles[i]->isBoundary) drawSphere(p, 0.1, glm::vec4(0, 1, 0, 0.8f));
		}
	}
};


#pragma once

#include "MeshGrid.hpp"
#include <algorithm>
#include <chrono>

#define PI 3.14159265f

void CalculateHashes(SPHSystem& sphSystem, const SPHSettings& settings) {
    for (auto& p : sphSystem.particles)
        p->hash = getHash(getCell(p, settings.h));
}

void SortParticles(SPHSystem& sphSystem) {
    std::sort(sphSystem.particles.begin(), sphSystem.particles.end(),
        [&](Particle* i,Particle* j) {
            return i->hash < j->hash;
        }
    );
}

void DensityAndPressures( SPHSystem& sphSystem, const uint32_t* particleTable, const SPHSettings& settings ) {
    for (auto piIndex = 0; piIndex < sphSystem.particleCount; piIndex++) {
        Particle* pi = sphSystem.particles[piIndex];
        if (pi->isBoundary) continue;
        float pDensity = 0;
        glm::ivec3 cell = getCell(pi, settings.h);

		for (int x = -1; x <= 1; x++) for (int y = -1; y <= 1; y++) for (int z = -1; z <= 1; z++) {
        //for (int x = -10; x <= 10; x++) for (int y = -10; y <= 10; y++) for (int z = -10; z <= 10; z++) {
            //if (x * x + y * y + z * z > 100) continue;
            uint32_t cellHash = getHash(cell + glm::ivec3(x, y, z));
			uint32_t pjIndex = particleTable[cellHash];
			if (pjIndex == NO_PARTICLE) {
				continue;
			}
			while (pjIndex < sphSystem.particleCount) {
				if (pjIndex == piIndex) {
					pjIndex++;
					continue;
				}
				Particle* pj = sphSystem.particles[pjIndex];
				if (pj->hash != cellHash) {
					break;
				}
                auto diff = length(pi->position - pj->position);
                
                if (0 <= diff && diff <= settings.h)
                    pDensity += settings.mass * settings.poly6 * (settings.h2 - diff * diff)* (settings.h2 - diff * diff)* (settings.h2 - diff * diff);
                pjIndex++;
			}
		}
        pi->density = pDensity + settings.restDensity;
        float pPressure = settings.gasConstant * pDensity;
        pi->pressure = pPressure;
    }
}

void CalculateForces( SPHSystem& sphSystem, const uint32_t* particleTable, const SPHSettings& settings, bool isSlime ) {
    for (auto piIndex = 0; piIndex < sphSystem.particleCount; piIndex++) {
        Particle* pi = sphSystem.particles[piIndex];
        if (pi->isBoundary) continue;
        glm::vec3 force = glm::vec3(0);
        glm::vec3 pNormal = glm::vec3(0);
        float pColor = 0;
        glm::ivec3 cell = getCell(pi, settings.h);

		for (int x = -1; x <= 1; x++) for (int y = -1; y <= 1; y++) for (int z = -1; z <= 1; z++) {
        //for (int x = -10; x <= 10; x++) for (int y = -10; y <= 10; y++) for (int z = -10; z <= 10; z++) {
            //if (x * x + y * y + z * z > 100) continue;
            uint32_t cellHash = getHash(cell + glm::ivec3(x, y, z));
			uint32_t pjIndex = particleTable[cellHash];
			if (pjIndex == NO_PARTICLE) {
				continue;
			}
			while (pjIndex < sphSystem.particleCount) {

				if (pjIndex == piIndex) {
					pjIndex++;
					continue;
				}
				Particle* pj = sphSystem.particles[pjIndex];
				if (pj->hash != cellHash) {
					break;
				}

				float diff = length(pi->position - pj->position);
                glm::vec3 dir = glm::normalize(pi->position - pj->position);
                glm::vec3 diffVec = pi->position - pj->position;

                if (0 <= diff && diff <= settings.h) {
                    float kernelGrad = settings.spikyGrad * (settings.h - diff) * (settings.h - diff);
                    float kernelLap = settings.spikyLap * (settings.h - diff);
                    // pressure force
                    glm::vec3 pressureForce =
                        -dir * settings.mass * (pi->pressure + pj->pressure) / (2 * pj->density) * kernelGrad;
                    
                    // viscosity force
                    glm::vec3 viscosityForce =
                        settings.viscosity * settings.mass * (pj->velocity - pi->velocity) / pj->density * kernelLap;

                    // surface tension
                    pColor += settings.mass / pj->density * settings.poly6 * pow((settings.h2 - diff * diff), 3);
                    glm::vec3 normal = dir * settings.mass / pj->density * kernelGrad;
                    pNormal += normal;
                    float kappa = 0;
                    if(length(normal) > 0.0001)
                        kappa = settings.mass / pj->density * kernelLap / length(normal);
                    glm::vec3 surfaceForce = settings.surfaceTension * kappa * normal;

                    force += pressureForce + viscosityForce + surfaceForce;
                }
				pjIndex++;
			}
		}
        //std::cout << length(pNormal) << std::endl;
        pi->color = pColor;
        pi->normal = pNormal;

        glm::vec3 gravityForce = glm::vec3(0, settings.g, 0) * settings.mass;

        // rewind force
        if (isSlime) {
            glm::vec3 target = settings.target;
            glm::vec3 dir = normalize(target - pi->position);

            float dist = length(target - pi->position);
            glm::vec3 slimeForce = settings.rewindConstant * dist * dist * dist * dir;

            force += slimeForce + glm::vec3(0, 9, 0) * settings.mass;;
        }

        pi->force = force + gravityForce;
        
    }
}

void UpdateParticlePositions( SPHSystem& sphSystem, const SPHSettings& settings, const float& deltaTime, const bool isSlime ) {
    float boxWidth = settings.boundary;
    float elasticity = 0.1;
    for (auto i = 0; i < sphSystem.particleCount; i++) {
        Particle* p = sphSystem.particles[i];
        if (p->isBoundary) continue;

        p->velocity += p->force / settings.mass * deltaTime;
        if (abs(p->velocity.x) * deltaTime > settings.h * 2 / 3)
            p->velocity.x = glm::sign(p->velocity.x) * (settings.h * 2 / 3) / deltaTime;
        if (abs(p->velocity.y) * deltaTime > settings.h * 2 / 3)
            p->velocity.y = glm::sign(p->velocity.y) * (settings.h * 2 / 3) / deltaTime;
        if (abs(p->velocity.z) * deltaTime > settings.h * 2 / 3)
            p->velocity.z = glm::sign(p->velocity.z) * (settings.h * 2 / 3) / deltaTime;
        //if (!isSlime && length(p->normal) < 0.002)
        //    p->velocity = glm::vec3(0);
        //

        p->position += p->velocity * deltaTime;


        // Floor Boundary
        if (p->position.y < 0.0001) {
            p->position.y = 0.0001;
            p->velocity.y *= -elasticity;
            p->velocity.x *= +elasticity;
            p->velocity.z *= +elasticity;
        }

        // Box Boundary
        //if (p->position.y > 2 * boxWidth - 0.0001) {
        //    p->position.y = 2 * boxWidth - 0.0001;
        //    p->velocity.y = -p->velocity.y * elasticity;
        //}
        //if (p->position.x < 0.0001) {
        //    p->position.x = 0.0001;
        //    p->velocity.x = -p->velocity.x * elasticity;
        //}
        //if (p->position.x > 2 * boxWidth - 0.0001) {
        //    p->position.x = 2 * boxWidth - 0.0001;
        //    p->velocity.x = -p->velocity.x * elasticity;
        //}
        //if (p->position.z < 0.0001) {
        //    p->position.z = 0.0001;
        //    p->velocity.z = -p->velocity.z * elasticity;
        //}
        //if (p->position.z > 2 * boxWidth - 0.0001) {
        //    p->position.z = 2 * boxWidth - 0.0001;
        //    p->velocity.z = -p->velocity.z * elasticity;
        //}
    }
}

void DetectSurface(const SPHSystem& sphSystem, const uint32_t* particleTable, const SPHSettings& settings) {
    for (auto i = 0; i < sphSystem.particleCount; i++) {
        Particle* pi = sphSystem.particles[i];
        pi->isClose = false;
        glm::ivec3 cell = getCell(pi, settings.h);
        bool isSurface = false;
        //std::cout << cell.x << " " << cell.y << " " << cell.z << " " << std::endl;
        for (int x = -1; x <= 1; x++) for (int y = -1; y <= 1; y++) for (int z = -1; z <= 1; z++) {
            uint32_t cellHash = getHash(cell + glm::ivec3(x, y, z));
            uint32_t pjIndex = particleTable[cellHash];
            //std::cout << x << " " << y << " " << z << " " << pjIndex << std::endl;
            if (pjIndex == NO_PARTICLE) {
                isSurface = true;
            }
        }
        if (isSurface) {
            pi->isClose = true;
            //std::cout << pi.x << " " << pi.y << " " << pi.z << " " << std::endl;
        }
    }
}

void UpdateParticles(SPHSystem& sphSystem, MeshGrid& meshGrid, const SPHSettings& settings, float deltaTime, bool isSlime) {
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();

    // Create Hash
    CalculateHashes(sphSystem, settings);
    SortParticles(sphSystem);
    uint32_t* particleTable = createNeighborTable(sphSystem, sphSystem.particleCount);
    
    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
    std::cout << "Simulation\t" << sec.count() * 1000.f << " ms" << std::endl;

    // Simulation
    DensityAndPressures(sphSystem, particleTable, settings);
    CalculateForces(sphSystem, particleTable, settings, isSlime);
    UpdateParticlePositions(sphSystem, settings, deltaTime, isSlime);



    start = std::chrono::system_clock::now();

    // Visualization
    meshGrid.ConstructCells(particleTable, sphSystem);
    meshGrid.MarchCube(sphSystem, particleTable, settings);

    sec = std::chrono::system_clock::now() - start;
    std::cout << "Visualization\t" << sec.count() * 1000.f << " ms" << std::endl;
    std::cout << std::endl;

    delete(particleTable);
}

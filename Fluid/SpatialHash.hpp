#pragma once
#include "SPHSystem.hpp"
#include "GLTools.hpp"

const uint32_t TABLE_SIZE = 262144;
const uint32_t NO_PARTICLE = 0xFFFFFFFF;

uint32_t getHash(const glm::ivec3& cell) {
    return ( (uint32_t)(cell.x * 73856093)
        ^ (uint32_t)(cell.y * 19349663)
        ^ (uint32_t)(cell.z * 83492791) ) % TABLE_SIZE;
}

//const uint32_t TABLE_SIZE = 1000;
//const uint32_t NO_PARTICLE = 0xFFFFFFFF;
//
//uint32_t getHash(const glm::ivec3& cell) {
//	return ((uint32_t)(cell.x * 29)
//		^ (uint32_t)(cell.y * 61)
//		^ (uint32_t)(cell.z * 97)) % TABLE_SIZE;
//}

glm::ivec3 getCell(Particle* p) {
	return { p->position.x, p->position.y, p->position.z };
}

glm::ivec3 getCell(Particle* p, float h) {
    return { p->position.x / h, p->position.y / h, p->position.z / h };
}

glm::ivec3 getCell(glm::vec3 p) {
	return { p.x * 10, p.y * 10, p.z * 10 };
}

glm::ivec3 getPos(Particle* p) {
	return { p->position.x / 10, p->position.y / 10, p->position.z / 10 };
}



//glm::ivec3 getCell(Particle* p, float h) {
//	return { p->position.x * 2, p->position.y * 2, p->position.z * 2 };
//}

uint32_t* createNeighborTable(SPHSystem& sphSystem, const size_t& particleCount) {
	uint32_t* particleTable
		= (uint32_t*)malloc(sizeof(uint32_t) * TABLE_SIZE);

	for (size_t i = 0; i < TABLE_SIZE; ++i) {
		particleTable[i] = NO_PARTICLE;
	}

	uint32_t prevHash = NO_PARTICLE;
	for (size_t i = 0; i < particleCount; ++i) {
		uint32_t currentHash = sphSystem.particles[i]->hash;
		if (currentHash != prevHash) {
			particleTable[currentHash] = i;
			prevHash = currentHash;
		}
	}
	return particleTable;
}

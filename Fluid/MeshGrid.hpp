#pragma once
#include "SpatialHash.hpp"
#include "MarchingCube.hpp"

float smin(float a, float b, float k) {
	k *= 1.0 / (1.0 - sqrt(0.5));
	float h = std::max(k - abs(a - b), 0.f) / k;
	return std::min(a, b) - k * 0.5 * (1.0 + h - sqrt(1.0 - h * (h - 2.0)));
}

struct MeshGrid {
private:
	std::vector<glm::vec3> cellPos;
	std::vector<glm::vec3> cellNormal;
	std::vector<float> cellVal;
	TRIANGLE tri;
	float xMin, xMax, yMin, yMax, zMin, zMax;
	float sdfConstant = 0.2;
	float radius = 0.5;
	float width = 0.5;

	int GetIndex(const glm::vec3 &pos) {
		int xLen = (xMax - xMin) / width; + 0.001;
		int yLen = (yMax - yMin) / width; + 0.001;
		int zLen = (zMax - zMin) / width; + 0.001;
		int x =   (pos.x - xMin) / width; + 0.001;
		int y =   (pos.y - yMin) / width; + 0.001;
		int z =   (pos.z - zMin) / width; + 0.001;
		return z + (y * (zLen+1)) + (x * (zLen+1) * (yLen+1));
	}

	void GetGridCell(const glm::vec3& pos, GRIDCELL &cell) {
		// Hard coding to order vertex with the marching cube reference
		int index	= GetIndex(glm::vec3(pos.x		  , pos.y		 , pos.z		));
		cell.p[0]	= cellPos[index];
		cell.val[0] = cellVal[index];

		index		= GetIndex(glm::vec3(pos.x + width, pos.y		 , pos.z		));
		cell.p[1]	= cellPos[index];
		cell.val[1] = cellVal[index];

		index		= GetIndex(glm::vec3(pos.x + width, pos.y		 , pos.z + width));
		cell.p[2]	= cellPos[index];
		cell.val[2] = cellVal[index];

		index		= GetIndex(glm::vec3(pos.x		  , pos.y		 , pos.z + width));
		cell.p[3]	= cellPos[index];
		cell.val[3] = cellVal[index];

		index		= GetIndex(glm::vec3(pos.x		  , pos.y + width, pos.z		));
		cell.p[4]	= cellPos[index];
		cell.val[4] = cellVal[index];
																					
		index		= GetIndex(glm::vec3(pos.x + width, pos.y + width, pos.z		));
		cell.p[5]	= cellPos[index];
		cell.val[5] = cellVal[index];

		index		= GetIndex(glm::vec3(pos.x + width, pos.y + width, pos.z + width));
		cell.p[6]	= cellPos[index];
		cell.val[6] = cellVal[index];

		index		= GetIndex(glm::vec3(pos.x		  , pos.y + width, pos.z + width));
		cell.p[7]	= cellPos[index];
		cell.val[7] = cellVal[index];
	}

	void CalcVertexNormal() {
		for (int i = 0; i < tri.tempN.size(); i++){
			glm::vec3 normal = glm::vec3(0);
			for (int j = 0; j < tri.tempN.size(); j++) {
				if (length(tri.v[i] - tri.v[j]) < 0.001f) normal += tri.tempN[j];
			}
			tri.n.push_back(normalize(normal));
		}
	}

	void FindBoundary(const SPHSystem& sphSystem, float& xMin, float& xMax, float& yMin, float& yMax, float& zMin, float& zMax) {
		xMin = sphSystem.particles[1]->position.x;
		xMax = xMin;
		yMin = sphSystem.particles[1]->position.y;
		yMax = yMin;
		zMin = sphSystem.particles[1]->position.z;
		zMax = zMin;

		for (auto& p : sphSystem.particles) {
			if (isnan(p->position.x)) continue;
			xMin = (p->position.x < xMin) ? p->position.x : xMin;
			xMax = (p->position.x > xMax) ? p->position.x : xMax;
			yMin = (p->position.y < yMin) ? p->position.y : yMin;
			yMax = (p->position.y > yMax) ? p->position.y : yMax;
			zMin = (p->position.z < zMin) ? p->position.z : zMin;
			zMax = (p->position.z > zMax) ? p->position.z : zMax;
		}
		xMin = (floor(xMin / width)) * width - 3*radius;
		yMin = (floor(yMin / width)) * width - 3*radius;
		zMin = (floor(zMin / width)) * width - 3*radius;
		xMax = (floor(xMax / width)) * width + 3*radius;
		yMax = (floor(yMax / width)) * width + 3*radius;
		zMax = (floor(zMax / width)) * width + 3*radius;
	}										   


public:
	void ConstructCells(const uint32_t* particleTable, const SPHSystem& sphSystem) {
		FindBoundary(sphSystem, xMin, xMax, yMin, yMax, zMin, zMax);
		cellPos.clear();
		cellVal.clear();
		cellNormal.clear();
		for (float x = xMin; x <= xMax; x += width) for (float y = yMin; y <= yMax; y += width) for (float z = zMin; z <= zMax; z += width) {
			glm::vec3 pos = glm::vec3(x, y, z);
			float preMin = length(sphSystem.particles[1]->position - pos) - radius;
			for (auto& p : sphSystem.particles) {
				if (isnan(p->position.x)) continue;
				preMin = smin(preMin, length(p->position - pos) - radius, sdfConstant);
			}
			cellPos.push_back(pos);
			cellVal.push_back(preMin);
		}
	}

	void MarchCube(SPHSystem& sphSystem, const uint32_t* particleTable, const SPHSettings& settings) {
		tri.v.clear();
		tri.n.clear();
		tri.f.clear();
		tri.tempN.clear();

		for (float x = xMin; x < xMax; x += width) for (float y = yMin; y < yMax; y += width) for (float z = zMin; z < zMax; z += width) {
			GRIDCELL cell;
			GetGridCell(glm::vec3(x, y, z), cell);
			//for (auto& c : cell.p) {
			//	if(length(cell.p[0] - c) > 1) drawSphere(cell.p[0], 0.1f);
			//}
			Polygonise(cell, 0, tri);
		}

		CalcVertexNormal();

		//for (auto& p : tri.v) {
		//	glm::ivec3 cell = getCell(p);
		//	glm::vec3 normal = glm::vec3(0);
		//
		//	for (int x = -15; x <= 15; x++) for (int y = -15; y <= 15; y++) for (int z = -15; z <= 15; z++) {
		//		//if (x * x + y * y + z * z > 100) continue;
		//		uint32_t cellHash = getHash(cell + glm::ivec3(x, y, z));
		//		uint32_t pjIndex = particleTable[cellHash];
		//		if (pjIndex == NO_PARTICLE) {
		//			continue;
		//		}
		//		while (pjIndex < sphSystem.particleCount) {
		//			Particle* pj = sphSystem.particles[pjIndex];
		//			if (pj->hash != cellHash) {
		//				break;
		//			}
		//			float diff = length(p - pj->position);
		//			if (0 <= diff && diff <= 2*settings.h) {
		//				normal += pj->normal * (settings.h - diff) * (settings.h - diff);
		//			}
		//			pjIndex++;
		//		}
		//	}
		//	tri.n.push_back(normalize(normal));
		//}
	}

	void DrawMeshes() {
		drawCylinder(glm::vec3(xMax, yMax, zMax), glm::vec3(xMax, yMax, zMin), 0.01f);
		drawCylinder(glm::vec3(xMax, yMax, zMax), glm::vec3(xMax, yMin, zMax), 0.01f);
		drawCylinder(glm::vec3(xMax, yMax, zMax), glm::vec3(xMin, yMax, zMax), 0.01f);
		drawCylinder(glm::vec3(xMin, yMin, zMin), glm::vec3(xMin, yMin, zMax), 0.01f);
		drawCylinder(glm::vec3(xMin, yMin, zMin), glm::vec3(xMin, yMax, zMin), 0.01f);
		drawCylinder(glm::vec3(xMin, yMin, zMin), glm::vec3(xMax, yMin, zMin), 0.01f);
		drawCylinder(glm::vec3(xMin, yMax, zMax), glm::vec3(xMin, yMin, zMax), 0.01f);
		drawCylinder(glm::vec3(xMin, yMax, zMax), glm::vec3(xMin, yMax, zMin), 0.01f);
		drawCylinder(glm::vec3(xMax, yMax, zMin), glm::vec3(xMin, yMax, zMin), 0.01f);
		drawCylinder(glm::vec3(xMax, yMax, zMin), glm::vec3(xMax, yMin, zMin), 0.01f);
		drawCylinder(glm::vec3(xMax, yMin, zMax), glm::vec3(xMin, yMin, zMax), 0.01f);
		drawCylinder(glm::vec3(xMax, yMin, zMax), glm::vec3(xMax, yMin, zMin), 0.01f);

		//int index = 0;
		//for (auto& v : tri.v) {
		//	drawCylinder(v, v - tri.n[index++], 0.01);
		//}

		drawObject(tri.v, tri.n, tri.f, glm::vec3(0), glm::vec4(0.1, 0.8, 0.1, 0));
	}
};








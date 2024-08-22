#pragma once
#include "SPH_GPU.hpp"

#define GRID_WIDTH 16
#define MESH_NUM_WORK_GROUPS GRID_WIDTH * GRID_WIDTH * GRID_WIDTH / 8
#define GRID_SIZE GRID_WIDTH * GRID_WIDTH * GRID_WIDTH
#define WIDTH 0.5f

uint32_t compute_program_handle_mesh[3]{ 0, 0, 0 };
uint32_t packed_cells_buffer_handle = 0;

glm::vec4 tempVal[GRID_SIZE];
glm::vec4 tempPos[GRID_SIZE];

// ssbo sizes
constexpr ptrdiff_t cellPos_ssbo_size   = sizeof(glm::vec4) * GRID_SIZE;
constexpr ptrdiff_t cellVal_ssbo_size   = sizeof(glm::vec4) * GRID_SIZE;
constexpr ptrdiff_t vertex_ssbo_size    = sizeof(glm::vec4) * GRID_SIZE;
constexpr ptrdiff_t face_ssbo_size      = sizeof(glm::vec4) * GRID_SIZE;
constexpr ptrdiff_t normal_ssbo_size    = sizeof(glm::vec4) * GRID_SIZE;

constexpr ptrdiff_t packed_cells_buffer_size =
cellPos_ssbo_size + cellVal_ssbo_size + vertex_ssbo_size + face_ssbo_size + normal_ssbo_size;

// ssbo offsets
constexpr ptrdiff_t cellPos_ssbo_offset  = 0;
constexpr ptrdiff_t cellVal_ssbo_offset  = cellPos_ssbo_size;
constexpr ptrdiff_t vertex_ssbo_offset   = cellVal_ssbo_offset + cellVal_ssbo_size;
constexpr ptrdiff_t face_ssbo_offset     = vertex_ssbo_offset + vertex_ssbo_size;
constexpr ptrdiff_t normal_ssbo_offset   = face_ssbo_offset + face_ssbo_size;

void InitMeshGL() {
    uint32_t compute_shader_handle;
    compute_shader_handle = loadShader("SDF.comp", GL_COMPUTE_SHADER);
    compute_program_handle_mesh[0] = glCreateProgram();
    glAttachShader(compute_program_handle_mesh[0], compute_shader_handle);
    glLinkProgram(compute_program_handle_mesh[0]);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = loadShader("marchingCube.comp", GL_COMPUTE_SHADER);
    compute_program_handle_mesh[1] = glCreateProgram();
    glAttachShader(compute_program_handle_mesh[1], compute_shader_handle);
    glLinkProgram(compute_program_handle_mesh[1]);
    glDeleteShader(compute_shader_handle);

    std::vector<glm::vec4> initial_position;
    for (int x = -GRID_WIDTH / 2; x < GRID_WIDTH / 2; x++)
        for (int y = -GRID_WIDTH / 2; y < GRID_WIDTH / 2; y++)
            for (int z = -GRID_WIDTH / 2; z < GRID_WIDTH / 2; z++) {
        initial_position.push_back(glm::vec4(x, y, z, 1));
    }

    void* initial_data = std::malloc(packed_cells_buffer_size);
    std::memset(initial_data, 0, packed_cells_buffer_size);
    std::memcpy(initial_data, initial_position.data(), cellPos_ssbo_size);
    glGenBuffers(1, &packed_cells_buffer_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_cells_buffer_handle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, packed_cells_buffer_size, initial_data, GL_DYNAMIC_STORAGE_BIT);
    std::free(initial_data);
}

void CreateMesh() {
    // bindings
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_particles_buffer_handle);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, packed_particles_buffer_handle, position_ssbo_offset, position_ssbo_size);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_cells_buffer_handle);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, packed_cells_buffer_handle, cellPos_ssbo_offset, cellPos_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, packed_cells_buffer_handle, cellVal_ssbo_offset, cellVal_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, packed_cells_buffer_handle, vertex_ssbo_offset, vertex_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, packed_cells_buffer_handle, face_ssbo_offset, face_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 5, packed_cells_buffer_handle, normal_ssbo_offset, normal_ssbo_size);

    glUseProgram(compute_program_handle_mesh[0]);
    glDispatchCompute(MESH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	
    glUseProgram(compute_program_handle_mesh[1]);
    glDispatchCompute(MESH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_cells_buffer_handle);
	//glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, GRID_SIZE * sizeof(glm::vec4), tempPos);
    //glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, GRID_SIZE * sizeof(glm::vec4), GRID_SIZE * sizeof(glm::vec4), tempVal);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void renderMesh() {
    int i = 0;
    for (auto& p : tempPos) {
        //std::cout << p->position.x << " " << p->position.y << " " << p->position.z << std::endl;
        glm::vec4 pos = WIDTH * p + glm::vec4(5, 1, 5, 0);
        if(tempVal[i].x < 0 ) drawSphere(glm::vec3(pos.x, pos.y, pos.z), 0.1);
        //drawSphere(glm::vec3(p.x, p.y, p.z), 0.01);
        i++;
    }
    //drawSphere(glm::vec3(0), 0.01, glm::vec4(0, 1, 0, 0));
}
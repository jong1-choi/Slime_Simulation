#pragma once
#include "SPHSystem.hpp"
// reference -> https://github.com/multiprecision/sph_opengl/tree/master

#define SPH_NUM_PARTICLES 125
#define SPH_NUM_WORK_GROUPS 125

uint32_t compute_program_handle[3]{ 0, 0, 0 };
uint32_t packed_particles_buffer_handle = 0;

// ssbo sizes
constexpr ptrdiff_t position_ssbo_size  = sizeof(glm::vec4) * SPH_NUM_PARTICLES;
constexpr ptrdiff_t velocity_ssbo_size  = sizeof(glm::vec4) * SPH_NUM_PARTICLES;
constexpr ptrdiff_t force_ssbo_size     = sizeof(glm::vec4) * SPH_NUM_PARTICLES;
constexpr ptrdiff_t density_ssbo_size   = sizeof(glm::vec4) * SPH_NUM_PARTICLES;
constexpr ptrdiff_t pressure_ssbo_size  = sizeof(glm::vec4) * SPH_NUM_PARTICLES;

constexpr ptrdiff_t packed_buffer_size =
position_ssbo_size + velocity_ssbo_size + force_ssbo_size + density_ssbo_size + pressure_ssbo_size;

// ssbo offsets
constexpr ptrdiff_t position_ssbo_offset = 0;
constexpr ptrdiff_t velocity_ssbo_offset = position_ssbo_size;
constexpr ptrdiff_t force_ssbo_offset    = velocity_ssbo_offset + velocity_ssbo_size;
constexpr ptrdiff_t density_ssbo_offset  = force_ssbo_offset + force_ssbo_size;
constexpr ptrdiff_t pressure_ssbo_offset = density_ssbo_offset + density_ssbo_size;

static void CheckGLError() {
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        std::string error;
        switch (err) {
        case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
        case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
        case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
        case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
        default:                        error = "UNKNOWN_ERROR";          break;
        }
        err = glGetError();
    }
}

void InitSPHGL( SPHSystem& sphSystem ) {
    uint32_t compute_shader_handle;
    compute_shader_handle = loadShader("density_pressure.comp", GL_COMPUTE_SHADER);
    compute_program_handle[0] = glCreateProgram();
    glAttachShader(compute_program_handle[0], compute_shader_handle);
    glLinkProgram(compute_program_handle[0]);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = loadShader("force.comp", GL_COMPUTE_SHADER);
    compute_program_handle[1] = glCreateProgram();
    glAttachShader(compute_program_handle[1], compute_shader_handle);
    glLinkProgram(compute_program_handle[1]);
    glDeleteShader(compute_shader_handle);

    compute_shader_handle = loadShader("integration.comp", GL_COMPUTE_SHADER);
    compute_program_handle[2] = glCreateProgram();
    glAttachShader(compute_program_handle[2], compute_shader_handle);
    glLinkProgram(compute_program_handle[2]);
    glDeleteShader(compute_shader_handle);

    std::vector<glm::vec4> initial_position;
    for (auto& p : sphSystem.particles) {
        initial_position.push_back(glm::vec4(p->position,1));
    }
    void* initial_data = std::malloc(packed_buffer_size);
    std::memset(initial_data, 0, packed_buffer_size);
    std::memcpy(initial_data, initial_position.data(), position_ssbo_size);
    glGenBuffers(1, &packed_particles_buffer_handle);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_particles_buffer_handle);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, packed_buffer_size, initial_data, GL_DYNAMIC_STORAGE_BIT);
    std::free(initial_data);
}

void run_simulation(SPHSystem& sphSystem) {
    // bindings
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_particles_buffer_handle);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, packed_particles_buffer_handle, position_ssbo_offset, position_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, packed_particles_buffer_handle, velocity_ssbo_offset, velocity_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 2, packed_particles_buffer_handle, force_ssbo_offset, force_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, packed_particles_buffer_handle, density_ssbo_offset, density_ssbo_size);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 4, packed_particles_buffer_handle, pressure_ssbo_offset, pressure_ssbo_size);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    glUseProgram(compute_program_handle[0]);
    glDispatchCompute(SPH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glUseProgram(compute_program_handle[1]);
    glDispatchCompute(SPH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    glUseProgram(compute_program_handle[2]);
    glDispatchCompute(SPH_NUM_WORK_GROUPS, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glm::vec4 tempPos[SPH_NUM_PARTICLES];
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, packed_particles_buffer_handle);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, SPH_NUM_PARTICLES * sizeof(glm::vec4), tempPos);
    int i = 0;
    for (auto& p : sphSystem.particles) {
        //std::cout << p->position.x << " " << p->position.y << " " << p->position.z << std::endl;
        p->position = glm::vec3(tempPos[i].x, tempPos[i].y, tempPos[i].z);
        i++;
    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

}
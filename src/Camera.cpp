#include "Camera.h"
#include <random>

// Initialize static thread-local members
thread_local std::mt19937 Camera::rng(std::random_device{}());
thread_local std::uniform_real_distribution<float> Camera::distribution(0.0f, 1.0f);

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <array>

#include <memory>
#include <algorithm>

#include "external/glm/glm.hpp"
#include "external/glm/gtx/euler_angles.hpp"
#include "external/glm/gtc/quaternion.hpp"
#include <external/glm/gtc/type_ptr.hpp>

using s8 = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using f32 = float;
using f64 = double;

using v2 = glm::vec2;
using v3 = glm::vec3;
using v4 = glm::vec4;
using m3 = glm::mat3;
using m4 = glm::mat4;
using quat = glm::quat;

using String = std::string;

// Containers
template <typename T>
using Vector = std::vector<T>;
template <typename K, typename V>
using Map = std::map<K,V>;
template <typename K, typename V>
using UMap = std::unordered_map<K,V>;
template <typename T, size_t Size>
using Array = std::array<T, Size>;

// Smart pointers
template <typename T>
using UniquePtr = std::unique_ptr<T>;
template <typename T>
using SharedPtr = std::shared_ptr<T>;
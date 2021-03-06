#ifndef PCH_H
#define PCH_H

//GLFW Includes
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

//GLM Includes
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>

//// IMGUI includes
#include <imgui/imgui.h>
// #include <imgui/ImGuiObject.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_internal.h>

//Standard Library Includes
#include <stdexcept>
#include <cstdlib>
#include <cstdint>
#include <fstream>
#include <chrono>
#include <functional>
#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <array>
#include <optional>

//Engine Classes
#include "Time.h"
#include "Component.h"

//Structs
#include "AngleAxis.h"
#include "DebugShape.h"
#include "Light.h"
#include "QueueFamilyIndices.h"
#include "SwapChainSupportDetails.h"
#include "TransformData.h"
#include "UniformBufferObject.h"
#include "Vertex.h"
#include "CollisionData.h"
#include "ProjectionData.h"

//Enums
#include "MeshTypes.h"
#include "Controls.h"
#include "ColliderTypes.h"
#include "InputStates.h"
#include "PhysicsLayers.h"

//Classes
#include "PartitionTree.h"
#include "BinaryPartitionTree.h"

#endif //PCH_H
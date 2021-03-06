#include "pch.h"
#include "AABBCollider.h"

#include "EntityManager.h"
#include "DebugManager.h"

#pragma region Constructor

AABBCollider::AABBCollider(glm::vec3 extents)
{
    this->extents = extents;

    colliderType = ColliderTypes::AABB;
}

#pragma endregion

#pragma region Accessors

glm::vec3 AABBCollider::GetExtents()
{
    return extents;
}

std::vector<glm::vec3> AABBCollider::getNormals() 
{
	return faceNormals;
}

void AABBCollider::SetExtents(glm::vec3 value)
{
    extents = value;
}

#pragma endregion

#pragma region GenerateFromMesh

void AABBCollider::GenerateFromMesh(std::shared_ptr<Mesh> mesh)
{
    //Find the center of the mesh
    std::vector<Vertex> vertices = mesh->GetVertices();
	//std::vector<glm::vec3> faceNormals;
    glm::vec3 position = GetParentTransform()->GetOrientation() * vertices[0].position * GetParentTransform()->GetScale();
    glm::vec3 max = position;
    glm::vec3 min = position;

    for (int i = 1; i < vertices.size(); i++) {
        position = GetParentTransform()->GetOrientation() * vertices[i].position * GetParentTransform()->GetScale();

        if (position.x < min.x) {
            min.x = position.x;
        }
        else if (position.x > max.x) {
            max.x = position.x;
        }

        if (position.y < min.y) {
            min.y = position.y;
        }
        else if (position.y > max.y) {
            max.y = position.y;
        }

        if (position.z < min.z) {
            min.z = position.z;
        }
        else if (position.z > max.z) {
            max.z = position.z;
        }
    }

    glm::vec3 center = glm::vec3(min.x + max.x, min.y + max.y, min.z + max.z) * 0.5f;
    extents = max - center;

	
	vertices[0].position =  vertices[1].position - vertices[2].position;
	faceNormals.push_back(glm::normalize(vertices[0].position));
	vertices[1].position = vertices[2].position - vertices[3].position;
	faceNormals.push_back(glm::normalize(vertices[1].position));
	vertices[2].position = vertices[3].position - vertices[0].position;
	faceNormals.push_back(glm::normalize(vertices[2].position));
	vertices[3].position = vertices[0].position - vertices[1].position;
	faceNormals.push_back(glm::normalize(vertices[3].position));
	
    //Set scale to match the extents of the collider
    transform->SetScale(2.0f * extents);
    SetOffset(center);
    Update();
}

#pragma endregion

#pragma region Helper Methods

bool AABBCollider::ContainsPoint(glm::vec3 point)
{
    glm::vec3 distance = point - transform->GetPosition();
    return (abs(distance.x) < extents.x && abs(distance.y) < extents.y && abs(distance.z) < extents.z);
}

glm::vec3 AABBCollider::ClosestToPoint(glm::vec3 point)
{
    glm::vec3 closest;
    glm::vec3 distance = point - transform->GetPosition();

    //Check x axis
    if (distance.x < -extents.x) {
        closest.x = -extents.x;
    }
    else if (distance.x > extents.x) {
        closest.x = extents.x;
    }
    else {
        closest.x = distance.x;
    }

    //Check y axis
    if (distance.y < -extents.y) {
        closest.y = -extents.y;
    }
    else if (distance.y > extents.y) {
        closest.y = extents.y;
    }
    else {
        closest.y = distance.y;
    }

    //Check z axis
    if (distance.z < -extents.z) {
        closest.z = -extents.z;
    }
    else if (distance.z > extents.z) {
        closest.z = extents.z;
    }
    else {
        closest.z = distance.z;
    }

    closest += transform->GetPosition();
    return closest;
}

ProjectionData AABBCollider::ProjectOntoAxis(glm::vec3 axis)
{
    ProjectionData data = {};
    glm::vec3 center = transform->GetPosition();
    glm::vec3 corners[8] = {
        center + glm::vec3(extents.x, extents.y, extents.z),
        center + glm::vec3(extents.x, extents.y, -extents.z),
        center + glm::vec3(extents.x, -extents.y, extents.z),
        center + glm::vec3(extents.x, -extents.y, -extents.z),
        center + glm::vec3(-extents.x, extents.y, extents.z),
        center + glm::vec3(-extents.x, extents.y, -extents.z),
        center + glm::vec3(-extents.x, -extents.y, extents.z),
        center + glm::vec3(-extents.x, -extents.y, -extents.z),
    };

    float sqrLength = glm::dot(axis, axis);
    data.minMax = glm::vec2(glm::dot(corners[0], axis) / sqrLength, glm::dot(corners[0], axis) / sqrLength);
    data.minPoints = std::vector<glm::vec3>();
    data.minPoints.push_back(corners[0]);
    data.maxPoints = std::vector<glm::vec3>();
    data.maxPoints.push_back(corners[0]);

    for (int i = 1; i < 8; i++) {
        float projectionMult = glm::dot(corners[i], axis) / sqrLength;

        if (projectionMult < data.minMax.x) {
            data.minMax.x = projectionMult;
            data.minPoints.clear();
            data.minPoints.push_back(corners[i]);
        }
        else if (projectionMult == data.minMax.x) {
            data.minPoints.push_back(corners[i]);
        }
        else if (projectionMult > data.minMax.y) {
            data.minMax.y = projectionMult;
            data.maxPoints.clear();
            data.maxPoints.push_back(corners[i]);
        }
        else if (projectionMult == data.minMax.y) {
            data.maxPoints.push_back(corners[i]);
        }
    }

    return data;
}

glm::vec3 AABBCollider::FindSurfaceNormal(glm::vec3 surfacePoint)
{
    //TODO: Make sure surface point is actually on the surface of the shape

    glm::vec3 direction = surfacePoint - transform->GetPosition();
    glm::vec3 normal = glm::vec3(0, 0, 0);

    if (direction.x >= extents.x) {
        normal += glm::vec3(1, 0, 0);
    }
    else if (direction.x <= -extents.x) {
        normal += glm::vec3(-1, 0, 0);
    }

    if (direction.y >= extents.y) {
        normal += glm::vec3(0, 1, 0);
    }
    else if (direction.y <= -extents.y) {
        normal += glm::vec3(0, -1, 0);
    }

    if (direction.z >= extents.z) {
        normal += glm::vec3(0, 0, 1);
    }
    else if (direction.z <= -extents.z) {
        normal += glm::vec3(0, 0, -1);
    }

    return glm::normalize(normal);
}

void AABBCollider::DrawHandles()
{
    DebugManager::GetInstance()->DrawWireCube(transform->GetPosition(), glm::vec3(1.0f, 1.0f, 0.0f), extents * 2.0f, 0.0f);
}

#pragma endregion
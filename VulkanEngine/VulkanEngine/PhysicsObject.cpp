#include "pch.h"
#include "PhysicsObject.h"

#include "DebugManager.h"
#include "PhysicsManager.h"
#include "VulkanManager.h"


#include "Collider.h"
#include "SphereCollider.h"
#include "AABBCollider.h"
#include "ARBBCollider.h"
#define DebugInstance DebugManager::GetInstance()

#pragma region Constructor

void PhysicsObject::SortDimensions(void)
{
	std::sort(dimensionArray, dimensionArray + dimensionCount);
}

bool PhysicsObject::SharesDimension(std::shared_ptr<PhysicsObject> other)
{
	// return early to test framerate
	// return true;
	if (0 == dimensionCount) {
		if (0 == other->dimensionCount)
			return true;
	}
	
	for (size_t i = 0; i < dimensionCount; ++i) {
		for (size_t j = 0; j < other->dimensionCount; j++) {
			if (dimensionArray[i] == other->dimensionArray[j])
				return true; // As soon as we find one we know they share dimensions
		}
	}

	return false;
}

PhysicsObject::PhysicsObject(std::shared_ptr<Transform> transform, PhysicsLayers physicsLayer, ColliderTypes::ColliderTypes colliderType, float mass, bool affectedByGravity, bool alive)
{
	this->transform = transform;
	this->mass = mass;
	this->affectedByGravity = affectedByGravity;
	this->physicsLayer = physicsLayer;
	this->alive = alive;

	velocity = glm::vec3(0.0f, 0.0f, 0.0f);
	acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
	angularVelocity = AngleAxis();
	angularAcceleration = AngleAxis();

	switch (colliderType) {
	case ColliderTypes::Sphere:
		collider = std::make_shared<SphereCollider>();
		break;
	case ColliderTypes::AABB:
		collider = std::make_shared<AABBCollider>();
		break;
	case ColliderTypes::ARBB:
		collider = std::make_shared<ARBBCollider>();
		break;
	}

	colliderColor = glm::vec3(1, 1, 1);
	collider->SetParentTransform(transform);
	dimensionArray = nullptr;
	dimensionCount = 0;
}

#pragma endregion

#pragma region Accessors

float PhysicsObject::GetMass()
{
	return mass;
}

float PhysicsObject::getAngularVelocityF() 
{
	return fAngularVelocity;
}

void PhysicsObject::setAngluarVelocityF(float aV) 
{
	fAngularVelocity = aV;
}

void PhysicsObject::SetMass(float value)
{
	mass = value;
}

glm::vec3 PhysicsObject::GetAcceleration()
{
	return acceleration;
}

glm::vec3 PhysicsObject::GetVelocity()
{
	return velocity;
}

glm::vec3 PhysicsObject::GetVelocityAtPoint(glm::vec3 point)
{
	glm::vec3 direction = point - transform->GetPosition();
	float distance = glm::length(direction);
	direction /= distance;

	if (angularVelocity.angle == 0) { //There is no angular velocity to deal with
		return velocity;
	}

	glm::vec3 pointVelocity = glm::vec3(0, 0, 0);

	glm::vec3 tangentDirection = glm::normalize(glm::cross(angularVelocity.axis, direction));
	if (!(isnan(tangentDirection.x) || isnan(tangentDirection.y) || isnan(tangentDirection.z))) {
		pointVelocity = tangentDirection * distance * angularVelocity.angle;
	}

	//std::cout << "Point Velocity: (" << pointVelocity.x << ", " << pointVelocity.y << ", " << pointVelocity.z << ")" << std::endl;
	return velocity + pointVelocity;
}

void PhysicsObject::SetVelocity(glm::vec3 value)
{
	velocity = value;
}

glm::quat PhysicsObject::GetAngularVelocity()
{
	return angularVelocity.ToQuaternion();
}

void PhysicsObject::SetAngularVelocity(glm::quat value)
{
	angularVelocity = value;
}

PhysicsLayers PhysicsObject::GetPhysicsLayer()
{
	return physicsLayer;
}

std::shared_ptr<Transform> PhysicsObject::GetTransform()
{
	return transform;
}

void PhysicsObject::SetTransform(std::shared_ptr<Transform> value)
{
	transform = value;
}

bool PhysicsObject::GetAlive()
{
	return alive;
}

void PhysicsObject::SetAlive(bool value)
{
	alive = value;
}

std::shared_ptr<Collider> PhysicsObject::GetCollider()
{
	return collider;
}

void PhysicsObject::AddDimension(unsigned int d)
{
	if (ContainsDimension(d)) return;

	size_t* pTemp;
	pTemp = new size_t[dimensionCount + 1];
	if (dimensionArray)
	{
		memcpy(pTemp, dimensionArray, sizeof(size_t) * dimensionCount);
		delete[] dimensionArray;
		dimensionArray = nullptr;
	}
	pTemp[dimensionCount] = d;
	dimensionArray = pTemp;
	// dimensions.push_back(d);
	++dimensionCount;
	SortDimensions();
}

void PhysicsObject::RemoveDimension(unsigned int d)
{
	if (dimensionCount == 0) return;


	for (unsigned int i = 0; i < dimensionCount; i++) {
		if (d == dimensionArray[i]) {
			// swap this index with the last one then pop
			std::swap(dimensionArray[i], dimensionArray[dimensionCount - 1]);
			size_t* pTemp;
			pTemp = new size_t[dimensionCount - 1];
			if (dimensionArray)
			{
				memcpy(pTemp, dimensionArray, sizeof(size_t) * (dimensionCount - 1));
				delete[] dimensionArray;
				dimensionArray = nullptr;
			}
			dimensionArray = pTemp;
			--dimensionCount;
			SortDimensions();
			return;
		}
	}
}

bool PhysicsObject::ContainsDimension(unsigned int d)
{
	for (int i = 0; i < dimensionCount; i++) {
		if (d == dimensionArray[i]) {
			return true;
		}
	}
	return false;
}

void PhysicsObject::SetColliderColor(glm::vec3 c)
{
	colliderColor = c;
}


#pragma endregion

#pragma region Physics

void PhysicsObject::ApplyForce(glm::vec3 force, bool applyMass)
{
	if (physicsLayer == PhysicsLayers::Static) {
		std::cout << "Tried to apply force to static object" << std::endl;
		return;
	}

	if (applyMass) {
		force /= mass;
	}

	acceleration += force;
}

void PhysicsObject::ApplyForce(glm::vec3 force, glm::vec3 point, bool applyMass)
{
	if (physicsLayer == PhysicsLayers::Static) {
		std::cout << "Tried to apply force to static object" << std::endl;
		return;
	}

	if (applyMass) {
		force /= mass;
	}

	glm::vec3 direction = point - transform->GetPosition();
	glm::vec3 axis = glm::normalize(glm::cross(-direction, force));
	glm::vec3 perpendicularForce = force - ((glm::dot(direction, force) / glm::dot(direction, direction)) * direction);
	float angularForce = glm::length(perpendicularForce) * glm::length(direction);

	//Angular force is already accounting for mass, no need to double calculate
	if (angularForce > 0.001f) {
		ApplyTorque(glm::angleAxis(angularForce, axis), false);
	}

	acceleration += force;
}

void PhysicsObject::ApplyTorque(glm::quat torque, bool applyMass)
{
	if (applyMass) {
		angularAcceleration += AngleAxis(torque) * (1.0f / mass);
	}
	else {
		angularAcceleration += torque;
		if (torque.z < 0.0f) { angularAcceleration.negative = false; }
		else{ angularAcceleration.negative = true; }
	}
}

#pragma endregion

#pragma region Component

void PhysicsObject::Init()
{
}

void PhysicsObject::Update()
{
	float friction = 0.002f;
	if (alive) {
		if (affectedByGravity) {
			ApplyForce(PhysicsManager::GetInstance()->GetGravity() * PhysicsManager::GetInstance()->GetGravityDirection(), false);
		}

		//Apply acceleration
		velocity += acceleration * (float)(VulkanManager::GetInstance()->dt);
		acceleration = glm::vec3(0.0f, 0.0f, 0.0f);
		angularVelocity += angularAcceleration * (float)(VulkanManager::GetInstance()->dt);//glm::mix(angularVelocity, angularAcceleration * angularVelocity, Time::GetDeltaTime());
		angularAcceleration = AngleAxis();

		//Apply velocity
		transform->Translate(velocity * (float)(VulkanManager::GetInstance()->dt));

		transform->Rotate((angularVelocity * (float)(VulkanManager::GetInstance()->dt)).ToQuaternion()/*glm::mix(orientation, angularVelocity * orientation, Time::GetDeltaTime())*/);

		if (velocity.x > 0.005f) {
			velocity.x -= friction;
		}
		else if(velocity.x < -0.005) {
			velocity.x += friction;
		}

		if (velocity.x > -0.005f && velocity.x < 0.005f) {
			velocity.x = 0;
		}
		
	}

	//Update collider
	if (collider != nullptr) {
		collider->Update();
	}

	if (DebugInstance->GetDrawHandles()) {
		DrawHandles();
	}
}

#pragma endregion

#pragma region Debug

void PhysicsObject::DrawHandles()
{
	//Draw velocity
	DebugInstance->DrawLine(transform->GetPosition(), transform->GetPosition() + velocity, glm::vec3(1.0f, 1.0f, 0.0f), 0.0f);
	//Draw acceleration
	DebugInstance->DrawLine(transform->GetPosition() + velocity, transform->GetPosition() + velocity + acceleration, glm::vec3(1.0f, 0.0f, 0.0f), 0.0f);

	if (collider != nullptr) {
		collider->DrawHandles();
	}
}

#pragma endregion
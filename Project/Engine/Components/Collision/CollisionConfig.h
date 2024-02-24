#pragma once
#include <cstdint>

//CollisionAttribute
const uint32_t kCollisionAttributePlayer = 0b00001;
const uint32_t kCollisionAttributeEnemy = 0b00010;
const uint32_t kCollisionAttributeFloor = 0b00100;
const uint32_t kCollisionAttributeGoal = 0b01000;
const uint32_t kCollisionAttributeWeapon = 0b10000;

//CollisionMask
const uint32_t kCollisionMaskPlayer = 0b11110;
const uint32_t kCollisionMaskEnemy = 0b11101;
const uint32_t kCollisionMaskFloor = 0b11011;
const uint32_t kCollisionMaskGoal = 0b00001;
const uint32_t kCollisionMaskWeapon = 0b00010;

//形状
const uint32_t kCollisionPrimitiveSphere = 0b1;
const uint32_t kCollisionPrimitiveAABB = kCollisionPrimitiveSphere << 1;
const uint32_t kCollisionPrimitiveOBB = kCollisionPrimitiveAABB << 1;
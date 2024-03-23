#pragma once
#include <cstdint>

//CollisionAttribute
const uint32_t kCollisionAttributePlayer = 0b00001;
const uint32_t kCollisionAttributeEnemy = 0b00010;
const uint32_t kCollisionAttributeWeapon = 0b00100;
const uint32_t kCollisionAttributeMissile = 0b01000;
const uint32_t kCollisionAttributeLaser = 0b10000;

//CollisionMask
const uint32_t kCollisionMaskPlayer = 0b11010;
const uint32_t kCollisionMaskEnemy = 0b00101;
const uint32_t kCollisionMaskWeapon = 0b01010;
const uint32_t kCollisionMaskMissile = 0b00101;
const uint32_t kCollisionMaskLaser = 0b00001;

//形状
const uint32_t kCollisionPrimitiveSphere = 0b1;
const uint32_t kCollisionPrimitiveAABB = kCollisionPrimitiveSphere << 1;
const uint32_t kCollisionPrimitiveOBB = kCollisionPrimitiveAABB << 1;
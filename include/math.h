#pragma once

typedef struct{
	float x;
	float y;
} m_v2;

typedef struct{
	float x;
	float y;
	float z;
} m_v3;

typedef struct{
	float x;
	float y;
	float z;
	float w;
} m_v4;

struct m_v3 Vector_Add(const struct m_v3 v1, const struct m_v3 v2);

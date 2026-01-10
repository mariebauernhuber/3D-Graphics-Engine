#pragma once

struct vec3d{ float x,y,z,w = 1; };
struct vec2d{ float x,y; };
struct triangle{ vec3d p[3]; };
struct mat4x4{ float m[4][4] = { {0} }; };

vec3d Matrix_MultiplyVector(const mat4x4 &m, const vec3d &i);
mat4x4 Matrix_MakeIdentity();
vec3d Vector_Add(const vec3d &v1, const vec3d &v2);
vec3d Vector_Sub(const vec3d &v1, const vec3d &v2);
vec3d Vector_Mul(const vec3d &v1, float k);
vec3d Vector_Div(const vec3d &v1, float k);
float Vector_DotProduct(const vec3d &v1, const vec3d &v2);
float Vector_Length(const vec3d &v);
vec3d Vector_Normalise(const vec3d &v);
vec3d Vector_CrossProduct(const vec3d &v1, const vec3d &v2);
mat4x4 Matrix_MakeRotationX(float fAngleRad);
mat4x4 Matrix_MakeRotationY(float fAngleRad);
mat4x4 Matrix_MakeRotationZ(float fAngleRad);
mat4x4 Matrix_MakeTranslation(float x, float y, float z);
mat4x4 Matrix_MakeProjection(float fFovDegrees, float fAspectRatio, float fNear, float fFar);
mat4x4 Matrix_MultiplyMatrix(mat4x4 &m1, mat4x4 &m2);
mat4x4 Matrix_PointAt(const vec3d &pos, const vec3d &target, const vec3d &up);
mat4x4 Matrix_QuickInverse(mat4x4 &m);
vec3d Vector_IntersectPlane(vec3d &plane_p, vec3d &plane_n, vec3d &lineStart, vec3d &lineEnd);
int Triangle_ClipAgainstPlane(vec3d plane_p, vec3d plane_n,triangle &in_tri, triangle &out_tri1, triangle &out_tri2);

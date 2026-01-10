#include "../include/mesh.hpp"
#include "../include/geometry.hpp"

    mat4x4 Object3D::GetWorldMatrix()  {
        mat4x4 matRotX = Matrix_MakeRotationX(rotation.x);
        mat4x4 matRotY = Matrix_MakeRotationY(rotation.y);
        mat4x4 matRotZ = Matrix_MakeRotationZ(rotation.z);

        mat4x4 matWorld = Matrix_MultiplyMatrix(matRotZ, matRotX);
        matWorld = Matrix_MultiplyMatrix(matWorld, matRotY);
        mat4x4 matTrans = Matrix_MakeTranslation(position.x, position.y, position.z);
        matWorld = Matrix_MultiplyMatrix(matWorld, matTrans);
        return matWorld;
    }

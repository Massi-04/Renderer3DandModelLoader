#include "FMath.h"

#define VEC_TO_RAD(v) { DirectX::XMConvertToRadians(v.X), DirectX::XMConvertToRadians(v.Y), DirectX::XMConvertToRadians(v.Z) }
#define VEC_TO_XVEC(v) { v.X, v.Y, v.Z, 1.0f }

Vec3 FMath::GetForwardVector(Vec3 rotation)
{
    rotation = VEC_TO_RAD(rotation);

    Vec3 forward;

    forward.X = cos(rotation.X) * sin(rotation.Y);
    forward.Y = -sin(rotation.X);
    forward.Z = cos(rotation.X) * cos(rotation.Y);

    return forward;
}

Vec3 FMath::GetRightVector(Vec3 rotation)
{
    rotation = VEC_TO_RAD(rotation);

    Vec3 right;

    right.X = cos(rotation.Y);
    right.Y = 0.0f;
    right.Z = -sin(rotation.Y);

    return right;
}

Vec3 FMath::GetUpVector(Vec3 rotation)
{
    DirectX::XMVECTOR up = DirectX::XMVector3Cross(VEC_TO_XVEC(GetForwardVector(rotation)), VEC_TO_XVEC(GetRightVector(rotation)));

    return { up.m128_f32[0], up.m128_f32[1], up.m128_f32[2] };
}

Mat4 FMath::GetModelMatrix(Vec3 location, Vec3 rotation, Vec3 scale)
{
    return
    {
        DirectX::XMMatrixScaling(scale.X, scale.Y, scale.Z)
        *
        DirectX::XMMatrixRotationRollPitchYawFromVector(VEC_TO_RAD(rotation))
        *
        DirectX::XMMatrixTranslation(location.X, location.Y, location.Z)
    };
}

Mat4 FMath::GetViewMatrix(Vec3 location, Vec3 rotation)
{
    auto defaultView = DirectX::XMMatrixLookAtLH(VEC_TO_XVEC(location), VEC_TO_XVEC((GetForwardVector(rotation) + location)), VEC_TO_XVEC(GetUpVector(rotation)));
    auto zRotation = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotation.Z));

    return defaultView * zRotation;
}

Mat4 FMath::GetPerspectiveMatrix(float aspectRatio, float fov, float nearZ, float farZ)
{
    return DirectX::XMMatrixPerspectiveFovLH(DirectX::XMConvertToRadians(fov), aspectRatio, nearZ, farZ);
}

Mat4 FMath::GetMatrixTransposed(DirectX::XMMATRIX mat)
{
    return DirectX::XMMatrixTranspose(mat);
}

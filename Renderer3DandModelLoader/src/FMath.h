#pragma once

#include <DirectXMath.h>

struct Vec2
{
    float X, Y;

    inline Vec2 operator-()
    {
        return { -X, -Y };
    }

    inline Vec2 operator*(Vec2 v)
    {
        return { X * v.X, Y * v.Y };
    }

    inline Vec2 operator*(float f)
    {
        return { X * f, Y * f };
    }

    inline Vec2& operator+=(Vec2 v)
    {
        X += v.X;
        Y += v.Y;

        return *this;
    }
};

struct Vec3
{
    float X, Y, Z;

    inline Vec3 operator-()
    {
        return { -X, -Y, -Z };
    }

    inline Vec3 operator*(Vec3 v)
    {
        return { X * v.X, Y * v.Y, Z * v.Z };
    }

    inline Vec3 operator+(Vec3 v)
    {
        return { X + v.X, Y + v.Y, Z + v.Z };
    }

    inline Vec3 operator*(float f)
    {
        return { X * f, Y * f, Z * f };
    }

    inline Vec3& operator+=(Vec3 v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;

        return *this;
    }
};

struct Vec4
{
    float X, Y, Z, W;

    inline Vec4 operator-()
    {
        return { -X, -Y, -Z, -W };
    }

    inline Vec4 operator*(Vec4 v)
    {
        return { X * v.X, Y * v.Y, Z * v.Z, W * v.W };
    }

    inline Vec4 operator*(float f)
    {
        return { X * f, Y * f, Z * f, W * f };
    }

    inline Vec4& operator+=(Vec4 v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;
        W += v.W;

        return *this;
    }
};

using Mat4 = DirectX::XMMATRIX;

class FMath
{
public:
    FMath() = delete;

    static Vec3 GetForwardVector(Vec3 rotation);
    static Vec3 GetRightVector(Vec3 rotation);
    static Vec3 GetUpVector(Vec3 rotation);

    static Mat4 GetModelMatrix(Vec3 location, Vec3 rotation, Vec3 scale);
    static Mat4 GetViewMatrix(Vec3 camLocation, Vec3 camRotation);
    static Mat4 GetPerspectiveMatrix(float aspectRatio, float fov, float nearZ, float farZ);
    static Mat4 GetMatrixTransposed(DirectX::XMMATRIX mat);

};


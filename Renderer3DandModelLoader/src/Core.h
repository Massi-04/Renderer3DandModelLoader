#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

typedef std::string String;
typedef std::wstring WString;

extern HINSTANCE GInstance;
extern HWND GWnd;
extern bool GAppShouldRun;

void ShowAlert(const String& msg);
String OpenFileDialog(const char* filters);

#define checkf(Condition, Msg) { if(!(Condition)) { ShowAlert(String("Assertion failed: ").append(Msg));  __debugbreak(); } }
#define check(Condition) { checkf(Condition, "") }

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

struct Vertex
{
    Vertex()
    {
        Location = { 0.0f, 0.0f, 0.0f, 0.0f };
        Color = { 0.0f, 0.0f, 0.0f, 0.0f };
        TextureCoords = { 0.0f, 0.0f };
    }

    Vertex(Vec4 loc, Vec4 col, Vec2 texCoord)
    {
        Location = loc;
        Color = col;
        TextureCoords = texCoord;
    }

    Vec4 Location;
    Vec4 Color;
    Vec2 TextureCoords;
};

struct Transform
{
    Vec3 Location;
    Vec3 Rotation;
    Vec3 Scale;
};

struct Camera
{
    Vec3 Location;
    Vec3 Rotation;
    float FOV;
};
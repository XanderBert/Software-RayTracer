#pragma once
namespace dae
{
    struct Vector2
    {
        Vector2() = default;
        Vector2(float _x, float _y);

        float x{};
        float y{};
    };
}
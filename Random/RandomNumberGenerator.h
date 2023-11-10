#pragma once
#include <random>

class RandomNumberGenerator
{
public:
    template<typename T>
    static T GetRandomValue(T min, T max)
    {
        if constexpr (std::is_integral_v<T>)
        {
            std::uniform_int_distribution<T> distribution(min, max);
            return distribution(m_RandomEngine);
        }
        
        else if constexpr (std::is_floating_point_v<T>)
        {
            std::uniform_real_distribution<T> distribution(min, max);
            return distribution(m_RandomEngine);
        }
        
        //No integral or floating point type
        else
        {
            static_assert(std::is_integral_v<T> || std::is_floating_point_v<T>,
            "Invalid template argument for RandomNumberGenerator::getRandomValue");
            return T{};
        }
    }

private:
    inline static std::mt19937_64 m_RandomEngine{ std::random_device{}()};
};


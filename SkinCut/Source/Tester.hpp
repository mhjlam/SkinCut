#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Math.hpp"


namespace SkinCut
{
    class Cutter;

    class Tester
    {
    public:
        using SampleLocations = std::vector<std::pair<Math::Vector2, Math::Vector2>>;
        using Sample = std::tuple<std::wstring, Math::Vector2, Math::Vector2>;
        
        static void Test(std::shared_ptr<Cutter> cutter, Math::Vector2 resolution, Math::Vector2 window);

    private:
        static std::vector<Sample> CreateSamples(SampleLocations& locations, std::vector<float>& lengths, std::wstring setName);
        static void RunTest(std::shared_ptr<Cutter> cutter, std::vector<Sample>& samples, Math::Vector2& resolution, Math::Vector2& window, Math::Matrix& projection, Math::Matrix& view);
    };
}

#pragma once
#include <tuple>
#include <string>
#include <vec2.hpp>
#include <settings.hpp>

using OriginalPos = Vec2<int>;
using OriginalPositions = std::vector<std::vector<OriginalPos>>;

std::tuple<std::string, OriginalPositions> input_original_state(const std::string& path, const Settings& settings);


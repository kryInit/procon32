#pragma once
#include <tuple>
#include <string>
#include <pos.hpp>
#include <settings.hpp>

using OriginalPositions = std::vector<std::vector<Pos>>;

std::tuple<std::string, OriginalPositions> input_original_state(const std::string& path, const Settings& settings);
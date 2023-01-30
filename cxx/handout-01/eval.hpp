#pragma once

#include <unordered_map>

#include "parser.hpp"

int eval(const std::unique_ptr<Node>& node,
         const std::unordered_map<std::string, int>& variables);

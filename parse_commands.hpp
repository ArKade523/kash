#pragma once
#include "AST.hpp"
#include <sstream>
#include <string>
#include <memory>
#include <iostream>

std::unique_ptr<Node> parse_command(const std::string &input);

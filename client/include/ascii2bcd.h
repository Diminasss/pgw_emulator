#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <stdexcept>
#include <iostream>
#include "logger.h"

std::vector<uint8_t> ascii2bcd(std::string ascii);

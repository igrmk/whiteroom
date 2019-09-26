#pragma once

#include <spdlog/spdlog.h>

const std::string log_pattern = "%Y-%m-%d_%H:%M:%S.%e|A:%n|L:%L|T:%t| %v";

typedef std::shared_ptr<spdlog::logger> logger;

inline logger getlog() { return spdlog::get("con"); }

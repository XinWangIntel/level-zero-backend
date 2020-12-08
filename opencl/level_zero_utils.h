// Copyright 2020 Intel Corporation
#pragma once

#include <vector>
#include <array>

//#define CL_HPP_ENABLE_EXCEPTIONS
//#define CL_HPP_MINIMUM_OPENCL_VERSION 210
//#define CL_HPP_TARGET_OPENCL_VERSION 210

#include <level_zero/ze_api.h>
#include "utils/utils.hpp"
#include "test_harness/test_harness.hpp"
#include "utils/logging.hpp"


namespace pmlc::rt::level_zero {

std::vector<ze_device_handle_t> getSupportedDevices();

} // namespace pmlc::rt::level_zero

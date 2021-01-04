// Copyright 2020 Intel Corporation

#include <vector>

#include "pmlc/rt/level_zero/level_zero_utils.h"
#include "pmlc/util/logging.h"

int main(int argc, char** argv) {
  ze_result_t result = ZE_RESULT_NOT_READY;
  try {
    result = zeInit(0);
  } catch (std::exception& e) {
    IVLOG(1, "failed to init level zero driver, result:" << result << " info:" << e.what());
    throw;
  }
  std::vector<ze_driver_handle_t> drivers;
  try {
    drivers = pmlc::rt::level_zero::lzu::get_all_driver_handles();
  } catch (std::exception& e) {
    IVLOG(1, e.what());
    throw;
  }
  std::vector<std::pair<ze_driver_handle_t, ze_device_handle_t>> supportedDevices =
      pmlc::rt::level_zero::lzu::getSupportedDevices();
  if (supportedDevices.empty()) {
    IVLOG(1, "No supported level zero devices available");
    return 0;
  }

  IVLOG(1, "Available level zero devices: " << supportedDevices.size());
  for (auto& target : supportedDevices) {
    IVLOG(1, "  " << pmlc::rt::level_zero::lzu::get_device_properties(target.second).name);
  }
  return 0;
}

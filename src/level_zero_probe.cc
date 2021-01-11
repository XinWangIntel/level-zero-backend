// Copyright 2020 Intel Corporation

#include "level_zero_utils.hpp"
#include <iostream>
#include <vector>

int main(int argc, char** argv) {
  ze_result_t result = ZE_RESULT_NOT_READY;
  try {
    result = zeInit(0);
    if (result != ZE_RESULT_SUCCESS) {
      std::cout << "Function zeInit failed with result: " << lzu::to_string(result) << std::endl;
      return -1;
    }
  } catch (std::exception& e) {
    std::cout << "Function zeInit crashed with result: " << lzu::to_string(result) << " info: " << e.what()
              << std::endl;
    return -2;
  }
  std::vector<ze_driver_handle_t> drivers;
  try {
    lzu::get_all_driver_handles();
  } catch (std::exception& e) {
    std::cout << "Get level zero driver handle crashed with info: " << e.what() << std::endl;
    return -3;
  }
  std::vector<std::pair<ze_driver_handle_t, ze_device_handle_t>> supportedDevices = lzu::getSupportedDevices();
  if (supportedDevices.empty()) {
    std::cout << "No supported level zero devices available" << std::endl;
    return -4;
  }

  std::cout << "Available level zero devices count: " << supportedDevices.size() << std::endl;
  for (auto& target : supportedDevices) {
    std::cout << "Device: " << lzu::get_device_properties(target.second).name << std::endl;
  }
  return 0;
}

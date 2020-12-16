#include <level_zero/ze_api.h>

#include <array>

#include "test_harness/test_harness.hpp"
#include "utils/logging.hpp"
#include "utils/utils.hpp"

#include <iostream>

namespace lzt = level_zero_tests;

const size_t size = 9;

struct copy_data {
  uint32_t *data;
};

int main(int argc, char **argv) {
  std::cout << "xin:" << __LINE__ << std::endl;
  ze_result_t result = zeInit(0);
  std::cout << "xin:" << __LINE__ << std::endl;
  if (result) {
    throw std::runtime_error("zeInit failed: " +
                              result);
  }

   ze_memory_type_t memory_type = ZE_MEMORY_TYPE_HOST;
  //ze_memory_type_t memory_type = ZE_MEMORY_TYPE_SHARED;
  int offset = 0;

  std::vector<ze_device_handle_t> supportedDevices;
  for (auto driver : lzt::get_all_driver_handles()) {
    for (auto device : lzt::get_devices(driver)) {
      supportedDevices.push_back(device);
    }
  }
  std::cout << "xin supportedDevice.size:" << supportedDevices.size() << std::endl;
  ze_context_handle_t context1 = lzt::get_default_context();
  ze_device_handle_t device1 = supportedDevices[0];
  ze_context_handle_t context = context1;
  ze_device_handle_t device = device1;
  ze_command_queue_handle_t command_queue = lzt::create_command_queue(context, device, 0,
                                    ZE_COMMAND_QUEUE_MODE_DEFAULT,
                                    ZE_COMMAND_QUEUE_PRIORITY_NORMAL, 0);
  ze_command_list_handle_t command_list = lzt::create_command_list(context, device, 0, 0);
  std::vector<uint8_t> binary_file = lzt::load_binary_file("spirv_0");
  ze_module_handle_t module = lzt::create_module(context,
                                                 device,
                                                 binary_file.data(),
                                                 binary_file.size(),
                                                 ZE_MODULE_FORMAT_IL_SPIRV,
                                                 "",
                                                 nullptr);
  
  //auto module = lzt::create_module(device, "spirv_0");
  ze_kernel_handle_t kernel = lzt::create_function(module, "main_kernel");
  
  {
    {

      int64_t *input_data, *input_data1, *output_data;
      if (memory_type == ZE_MEMORY_TYPE_HOST) {
        input_data = static_cast<int64_t *>(
            lzt::allocate_host_memory(size * sizeof(int64_t)));
        input_data1 = static_cast<int64_t *>(
            lzt::allocate_host_memory(size * sizeof(int64_t)));
        output_data = static_cast<int64_t *>(
            lzt::allocate_host_memory(size * sizeof(int64_t)));
      } else {
        input_data = static_cast<int64_t *>(
            lzt::allocate_shared_memory(size * sizeof(int64_t)));
        input_data1 = static_cast<int64_t *>(
            lzt::allocate_shared_memory(size * sizeof(int64_t)));
        output_data = static_cast<int64_t *>(
            lzt::allocate_shared_memory(size * sizeof(int64_t)));
      }
      std::cout << "xin:" << __LINE__ << std::endl;

      // for(int i = 0; i < size; i++) {
      //    input_data[i] = i;
      //    input_data1[i] = i;
     // }
      std::cout << "xin:" << __LINE__ << std::endl;
      lzt::zeEventPool eventPool;
      eventPool.InitEventPool(context, 32);
      std::vector<uint64_t> value0 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
      ze_event_handle_t e0;
      eventPool.create_event(e0);
      lzt::append_memory_copy(command_list, (void *)input_data,
                              (void *)value0.data(), 9 * 8, e0, 0, nullptr);

      std::cout << "xin:" << __LINE__ << std::endl;
      std::vector<uint64_t> value1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
      ze_event_handle_t e1;
      eventPool.create_event(e1);
      lzt::append_memory_copy(command_list, (void *)input_data1,
                              (void *)value1.data(), 9 * 8, e1, 0, nullptr);

      std::vector<uint64_t> out = {0, 0, 0, 0, 0, 0, 0, 0, 0};
      ze_event_handle_t e2;
      eventPool.create_event(e2);
      lzt::append_memory_copy(command_list, (void *)output_data,
                              (void *)out.data(), 9 * 8, e2, 0, nullptr);

      std::cout << "xin:" << __LINE__ << std::endl;
      std::cout << "xin: input_data size:" << sizeof(input_data) << std::endl;
      ze_kernel_handle_t kernel1 = kernel;
      lzt::set_argument_value(kernel1, 0, sizeof(input_data), &input_data);
      lzt::set_argument_value(kernel1, 1, sizeof(input_data1), &input_data1);
      lzt::set_argument_value(kernel1, 2, sizeof(output_data), &output_data);

      std::cout << "xin:" << __LINE__ << std::endl;
      lzt::set_group_size(kernel, 1, 9, 9);

      ze_group_count_t group_count;
      group_count.groupCountX = 1;
      group_count.groupCountY = 9;
      group_count.groupCountZ = 9;

      std::vector<ze_event_handle_t> events;
      events.push_back(e0);
      events.push_back(e1);
      events.push_back(e2);
      ze_event_handle_t e3;
      eventPool.create_event(e3);
      lzt::append_launch_function(command_list, kernel, &group_count, e3,
                                  events.size(), events.data());

      std::cout << "xin:" << __LINE__ << std::endl;
      //lzt::close_command_list(command_list);
      //lzt::execute_command_lists(command_queue, 1, &command_list, nullptr);
      //lzt::synchronize(command_queue, UINT64_MAX);
      
      ze_event_handle_t e0_0;
      eventPool.create_event(e0_0);
      std::vector<ze_event_handle_t> events0;
      events0.push_back(e3);
      events0.push_back(e0);
      lzt::append_memory_copy(command_list, (void *)value0.data(),
                              (void *)input_data, 9 * 8, e0_0, events0.size(),
                              events0.data());

      ze_event_handle_t e1_1;
      eventPool.create_event(e1_1);
      std::vector<ze_event_handle_t> events1;
      events1.push_back(e3);
      events1.push_back(e1);
      lzt::append_memory_copy(command_list, (void *)value1.data(),
                              (void *)input_data1, 9 * 8, e1_1, events1.size(),
                              events1.data());

      // std::vector<uint64_t> out = {0, 0, 0, 0, 0, 0};
      ze_event_handle_t e2_2;
      eventPool.create_event(e2_2);
      std::vector<ze_event_handle_t> events2;
      events2.push_back(e3);
      events2.push_back(e2);
      lzt::append_memory_copy(command_list, (void *)out.data(),
                              (void *)output_data, 9 * 8, e2_2, events2.size(),
                              events2.data());

      std::cout << "xin:" << __LINE__ << std::endl;
      lzt::close_command_list(command_list);
      lzt::execute_command_lists(command_queue, 1, &command_list, nullptr);
      lzt::synchronize(command_queue, UINT64_MAX);

      std::cout << "xin:" << __LINE__ << std::endl;
      // if (0 != memcmp(input_data, output_data + offset, (size - offset) *
      // sizeof(int)))
      //    return -1;

      ze_event_handle_t et0 = e0_0;
      ze_event_handle_t et1 = e1_1;
      ze_event_handle_t et2 = e2_2;
      zeEventHostSynchronize(et0, UINT64_MAX);
      zeEventHostSynchronize(et1, UINT64_MAX);
      zeEventHostSynchronize(et2, UINT64_MAX);
      zeEventHostSynchronize(e0_0, UINT64_MAX);
      zeEventHostSynchronize(e1_1, UINT64_MAX);
      zeEventHostSynchronize(e2_2, UINT64_MAX);

      for (int i = 0; i < size; i++) {
        std::cout << output_data[i] << " ";
      }
      std::cout << std::endl;
      for (int i = 0; i < size; i++) {
        std::cout << out[i] << " ";
      }

      lzt::synchronize(command_queue, UINT64_MAX);
      // cleanup
      lzt::free_memory(input_data);
      lzt::free_memory(output_data);

      eventPool.destroy_event(e0);
      eventPool.destroy_event(e1);
      eventPool.destroy_event(e2);
      eventPool.destroy_event(e3);
      eventPool.destroy_event(e0_0);
      eventPool.destroy_event(e1_1);
      eventPool.destroy_event(e2_2);

      lzt::destroy_function(kernel);
      lzt::destroy_module(module);
      lzt::destroy_command_list(command_list);
      lzt::destroy_command_queue(command_queue);
    }
    std::cout << std::endl;
  }
  return 0;
}

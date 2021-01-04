#include "test_harness/test_harness.hpp"
#include "utils/logging.hpp"
#include "utils/utils.hpp"
#include <array>
#include <level_zero/ze_api.h>

namespace lzt = level_zero_tests;

const size_t size = 8;

struct copy_data {
  uint32_t* data;
};

int main(int argc, char** argv) {
  ze_result_t result = zeInit(0);
  // if (result) {
  //  throw std::runtime_error("zeInit failed: " +
  //                            result);
  // }

  ze_memory_type_t memory_type = ZE_MEMORY_TYPE_HOST;
  // ze_memory_type_t memory_type = ZE_MEMORY_TYPE_SHARED;
  int offset = 0;
  for (auto driver : lzt::get_all_driver_handles()) {
    for (auto device : lzt::get_devices(driver)) {
      // set up
      auto command_queue = lzt::create_command_queue();
      auto command_list = lzt::create_command_list();

      auto module = lzt::create_module(device, "copy_module.spv");
      auto kernel = lzt::create_function(module, "copy_data");

      int *input_data, *output_data;
      if (memory_type == ZE_MEMORY_TYPE_HOST) {
        input_data = static_cast<int*>(lzt::allocate_host_memory(size * sizeof(int)));
        output_data = static_cast<int*>(lzt::allocate_host_memory(size * sizeof(int)));
      } else {
        input_data = static_cast<int*>(lzt::allocate_shared_memory(size * sizeof(int)));
        output_data = static_cast<int*>(lzt::allocate_shared_memory(size * sizeof(int)));
      }

      lzt::write_data_pattern(input_data, size * sizeof(int), 1);
      memset(output_data, 0, size * sizeof(int));

      lzt::set_argument_value(kernel, 0, sizeof(input_data), &input_data);
      lzt::set_argument_value(kernel, 1, sizeof(output_data), &output_data);
      lzt::set_argument_value(kernel, 2, sizeof(int), &offset);
      lzt::set_argument_value(kernel, 3, sizeof(int), &size);

      lzt::set_group_size(kernel, 1, 1, 1);

      ze_group_count_t group_count;
      group_count.groupCountX = 1;
      group_count.groupCountY = 1;
      group_count.groupCountZ = 1;

      lzt::append_launch_function(command_list, kernel, &group_count, nullptr, 0, nullptr);

      lzt::close_command_list(command_list);
      lzt::execute_command_lists(command_queue, 1, &command_list, nullptr);
      lzt::synchronize(command_queue, UINT64_MAX);

      if (0 != memcmp(input_data, output_data + offset, (size - offset) * sizeof(int))) return -1;

      // cleanup
      lzt::free_memory(input_data);
      lzt::free_memory(output_data);
      lzt::destroy_function(kernel);
      lzt::destroy_module(module);
      lzt::destroy_command_list(command_list);
      lzt::destroy_command_queue(command_queue);
    }
  }
  return 0;
}

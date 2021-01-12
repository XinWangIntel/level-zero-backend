#include <array>
#include <chrono>
#include <iostream>

#include "level_zero_utils.hpp"

const size_t size = 9;

struct copy_data {
  uint32_t* data;
};

int main(int argc, char** argv) {
  ze_result_t result = ZE_RESULT_NOT_READY;
  std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
  try {
    result = zeInit(0);
    if (result != ZE_RESULT_SUCCESS) {
      std::cout << "Function zeInit failed with result: " << lzu::to_string(result) << std::endl;
      return -1;
    }
  } catch (std::exception& e) {
    std::ostringstream ostr1;
    std::cout << "Function zeInit crashed with result: " << lzu::to_string(result) << " info: " << e.what()
              << std::endl;
    throw std::runtime_error(ostr1.str());
  }

  ze_memory_type_t memory_type = ZE_MEMORY_TYPE_HOST;
  // ze_memory_type_t memory_type = ZE_MEMORY_TYPE_SHARED;
  // ze_memory_type_t memory_type = ZE_MEMORY_TYPE_DEVICE;
  int offset = 0;

  std::vector<std::pair<ze_driver_handle_t, ze_device_handle_t>> supportedDevices = lzu::getSupportedDevices();
  if (supportedDevices.empty()) {
    std::cout << "No supported level zero devices available" << std::endl;
    return -2;
  }

  std::cout << "Available level zero devices count: " << supportedDevices.size() << std::endl;
  for (auto& target : supportedDevices) {
    std::cout << "Device: " << lzu::get_device_properties(target.second).name << std::endl;
  }

  ze_context_handle_t context = lzu::get_context(supportedDevices[0].first);
  ze_device_handle_t device = supportedDevices[0].second;
  ze_command_queue_handle_t command_queue = lzu::create_command_queue(
      context, device, /*flags*/ 0, ZE_COMMAND_QUEUE_MODE_DEFAULT, ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
      /*ordinal*/ 0, /*index*/ 0);
  ze_command_list_handle_t command_list = lzu::create_command_list(context, device, /*flags*/ 0, /*ordinal*/ 0);
  std::vector<uint8_t> binary_file = lzu::load_binary_file("spirv_0");
  ze_module_handle_t module = lzu::create_module(context, device, binary_file.data(), binary_file.size(),
                                                 ZE_MODULE_FORMAT_IL_SPIRV, "", nullptr);

  // auto module = lzu::create_module(device, "spirv_0");
  ze_kernel_handle_t kernel = lzu::create_function(module, /*flag*/ 0, "main_kernel");

  {
    {
      int64_t *input_data, *input_data1, *output_data;
      if (memory_type == ZE_MEMORY_TYPE_HOST) {
        // Can change host ptr data like
        // input_data[0] = 255;
        input_data = static_cast<int64_t*>(lzu::allocate_host_memory(size * sizeof(int64_t), 1, context));
        input_data1 = static_cast<int64_t*>(lzu::allocate_host_memory(size * sizeof(int64_t), 1, context));
        output_data = static_cast<int64_t*>(lzu::allocate_host_memory(size * sizeof(int64_t), 1, context));
      } else if (memory_type = ZE_MEMORY_TYPE_DEVICE) {
        input_data =
            static_cast<int64_t*>(lzu::allocate_device_memory(size * sizeof(int64_t), 1, 0, 0, device, context));
        input_data1 =
            static_cast<int64_t*>(lzu::allocate_device_memory(size * sizeof(int64_t), 1, 0, 0, device, context));
        output_data =
            static_cast<int64_t*>(lzu::allocate_device_memory(size * sizeof(int64_t), 1, 0, 0, device, context));
      } else {
        input_data =
            static_cast<int64_t*>(lzu::allocate_shared_memory(size * sizeof(int64_t), 1, 0, 0, device, context));
        input_data1 =
            static_cast<int64_t*>(lzu::allocate_shared_memory(size * sizeof(int64_t), 1, 0, 0, device, context));
        output_data =
            static_cast<int64_t*>(lzu::allocate_shared_memory(size * sizeof(int64_t), 1, 0, 0, device, context));
      }

      std::vector<ze_event_handle_t> allEvents;
      lzu::zeEventPool eventPool;
      eventPool.InitEventPool(context, 32);
      std::vector<uint64_t> value0 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
      ze_event_handle_t e0;
      eventPool.create_event(&e0);
      allEvents.push_back(e0);
      lzu::append_memory_copy(command_list, reinterpret_cast<void*>(input_data), reinterpret_cast<void*>(value0.data()),
                              9 * 8, e0, 0, nullptr);

      std::vector<uint64_t> value1 = {1, 2, 3, 4, 5, 6, 7, 8, 9};
      ze_event_handle_t e1;
      eventPool.create_event(&e1);
      allEvents.push_back(e1);
      lzu::append_memory_copy(command_list, reinterpret_cast<void*>(input_data1),
                              reinterpret_cast<void*>(value1.data()), 9 * 8, e1, 0, nullptr);

      std::vector<uint64_t> out = {0, 0, 0, 0, 0, 0, 0, 0, 0};
      ze_event_handle_t e2;
      eventPool.create_event(&e2);
      allEvents.push_back(e2);
      lzu::append_memory_copy(command_list, reinterpret_cast<void*>(output_data), reinterpret_cast<void*>(out.data()),
                              9 * 8, e2, 0, nullptr);

      ze_kernel_handle_t kernel1 = kernel;
      lzu::set_argument_value(kernel1, 0, sizeof(input_data), &input_data);
      lzu::set_argument_value(kernel1, 1, sizeof(input_data1), &input_data1);
      lzu::set_argument_value(kernel1, 2, sizeof(output_data), &output_data);

      // Group size and count will influence some old neo drivers on subgroup broadcast part.
      // Each group size
      lzu::set_group_size(kernel, 1, 9, 9);

      // Total group count
      ze_group_count_t group_count;
      group_count.groupCountX = 1;
      group_count.groupCountY = 9;
      group_count.groupCountZ = 9;

      std::vector<ze_event_handle_t> events;
      events.push_back(e0);
      events.push_back(e1);
      events.push_back(e2);
      ze_event_handle_t e3;
      eventPool.create_event(&e3);
      allEvents.push_back(e3);
      lzu::append_launch_function(command_list, kernel, &group_count, e3, events.size(), events.data());

      // For host kind memory, can submmit here and then copy by cpu.
      // For shared and device memory, can submit later
      // Event takes more time than cpu computation on some machine
      // lzu::close_command_list(command_list);
      // lzu::execute_command_lists(command_queue, 1, &command_list, nullptr);
      // lzu::synchronize(command_queue, UINT64_MAX);

      ze_event_handle_t e0_0;
      eventPool.create_event(&e0_0);
      allEvents.push_back(e0_0);
      std::vector<ze_event_handle_t> events0;
      events0.push_back(e3);
      events0.push_back(e0);
      lzu::append_memory_copy(command_list, reinterpret_cast<void*>(value0.data()), reinterpret_cast<void*>(input_data),
                              9 * 8, e0_0, events0.size(), events0.data());

      ze_event_handle_t e1_1;
      eventPool.create_event(&e1_1);
      allEvents.push_back(e1_1);
      std::vector<ze_event_handle_t> events1;
      events1.push_back(e3);
      events1.push_back(e1);
      lzu::append_memory_copy(command_list, reinterpret_cast<void*>(value1.data()),
                              reinterpret_cast<void*>(input_data1), 9 * 8, e1_1, events1.size(), events1.data());

      // std::vector<uint64_t> out = {0, 0, 0, 0, 0, 0};
      ze_event_handle_t e2_2;
      eventPool.create_event(&e2_2);
      allEvents.push_back(e2_2);
      std::vector<ze_event_handle_t> events2;
      events2.push_back(e3);
      events2.push_back(e2);
      lzu::append_memory_copy(command_list, reinterpret_cast<void*>(out.data()), reinterpret_cast<void*>(output_data),
                              9 * 8, e2_2, events2.size(), events2.data());

      lzu::close_command_list(command_list);
      lzu::execute_command_lists(command_queue, 1, &command_list, nullptr);
      lzu::synchronize(command_queue, UINT64_MAX);

      // Check the result
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

      std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
      std::chrono::duration<double> diff = end - start;
      std::cout << "All executation time: " << diff.count() << "ms" << std::endl;

      std::cout << "Output data: " << std::endl;
      for (int i = 0; i < size; i++) {
        std::cout << output_data[i] << " ";
      }
      std::cout << std::endl;
      std::cout << "out data: " << std::endl;
      for (int i = 0; i < size; i++) {
        std::cout << out[i] << " ";
      }
      std::cout << std::endl;

      // final confirm
      lzu::synchronize(command_queue, UINT64_MAX);

      // profiling
      using std::chrono::nanoseconds;
      using fp_milliseconds = std::chrono::duration<double, std::chrono::milliseconds::period>;
      nanoseconds totalExecuteTime{0};
      nanoseconds kernelExecuteTime{0};
      nanoseconds memoryExecuteTime{0};
      // unsigned kernelsCnt = 0;
      // unsigned memoryCnt = 0;

      auto getEventKernelTimestamp = [](ze_event_handle_t event) -> ze_kernel_timestamp_result_t {
        ze_kernel_timestamp_result_t value = {};
        zeEventQueryKernelTimestamp(event, &value);
        return value;
      };

      auto deviceProperties = lzu::get_device_properties(device);
      const uint64_t timestampFreq = deviceProperties.timerResolution;
      const uint64_t timestampMaxValue = ~(-1 << deviceProperties.kernelTimestampValidBits);

      for (auto event : allEvents) {
        if (event == nullptr) continue;
        ze_kernel_timestamp_result_t timestamp = getEventKernelTimestamp(event);
        uint64_t start = timestamp.context.kernelStart;
        uint64_t end = timestamp.context.kernelEnd;
        auto eventExecuteTime =
            (end >= start) ? (end - start) * timestampFreq : ((timestampMaxValue - start + end + 1) * timestampFreq);
        nanoseconds executeTime{eventExecuteTime};
        totalExecuteTime += executeTime;

        // if (event->getKind() == LevelZeroActionKind::Kernel) {
        // kernelExecuteTime += executeTime;
        // kernelsCnt += 1;
        // IVLOG(2, "  Kernel '" << event->getName() << "' execute time: "
        //                  << fp_milliseconds(executeTime).count() << "ms");
        //}
      }

      std::cout << "Total Level Zero execution time: " << (fp_milliseconds(totalExecuteTime).count()) << "ms"
                << std::endl;

      // cleanup
      lzu::free_memory(context, reinterpret_cast<void*>(input_data));
      lzu::free_memory(context, reinterpret_cast<void*>(input_data1));
      lzu::free_memory(context, reinterpret_cast<void*>(output_data));

      eventPool.destroy_event(e0);
      eventPool.destroy_event(e1);
      eventPool.destroy_event(e2);
      eventPool.destroy_event(e3);
      eventPool.destroy_event(e0_0);
      eventPool.destroy_event(e1_1);
      eventPool.destroy_event(e2_2);

      lzu::destroy_function(kernel);
      lzu::destroy_module(module);
      lzu::destroy_command_list(command_list);
      lzu::destroy_command_queue(command_queue);
    }
    std::cout << std::endl;
  }
  std::cout << "Finish." << std::endl;
  return 0;
}

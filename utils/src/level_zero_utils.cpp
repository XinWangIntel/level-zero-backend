// Copyright 2020 Intel Corporation

#include "level_zero_utils.hpp"

namespace lzu {

#define LEVEL_ZERO_ASSERT(x)                                 \
  {                                                          \
    if (!(x)) {                                              \
      std::ostringstream oss;                                \
      oss << "Failed in " << __func__ << " at " << __LINE__; \
      throw std::runtime_error(oss.str());                   \
    }                                                        \
  }
#define LEVEL_ZERO_EXPECT_EQ(x, y) LEVEL_ZERO_ASSERT((x) == (y))
#define LEVEL_ZERO_EXPECT_NE(x, y) LEVEL_ZERO_ASSERT((x) != (y))
#define LEVEL_ZERO_EXPECT_GT(x, y) LEVEL_ZERO_ASSERT((x) > (y))
#define LEVEL_ZERO_EXPECT_TRUE(x) LEVEL_ZERO_ASSERT((x))
#define LEVEL_ZERO_ERRSTR(_err_) \
  if (result == _err_) return #_err_;

std::vector<std::pair<ze_driver_handle_t, ze_device_handle_t>> getSupportedDevices() {
  std::vector<std::pair<ze_driver_handle_t, ze_device_handle_t>> supportedDevices;
  for (auto driver : lzu::get_all_driver_handles()) {
    for (auto device : lzu::get_devices(driver)) {
      supportedDevices.push_back(std::make_pair(driver, device));
    }
  }
  return supportedDevices;
}

// Context
ze_context_handle_t get_context(ze_driver_handle_t driver) {
  ze_result_t result = ZE_RESULT_SUCCESS;

  ze_context_handle_t context = nullptr;
  ze_context_desc_t context_desc = {};
  context_desc.stype = ZE_STRUCTURE_TYPE_CONTEXT_DESC;
  result = zeContextCreate(driver, &context_desc, &context);

  if (ZE_RESULT_SUCCESS != result) {
    throw std::runtime_error("zeContextCreate failed: " + to_string(result));
  }

  return context;
}

void destroy_context(ze_context_handle_t context) {
  LEVEL_ZERO_EXPECT_TRUE(ZE_RESULT_SUCCESS == zeContextDestroy(context));
}

// Driver
uint32_t get_driver_handle_count() {
  uint32_t count = 0;
  ze_result_t result = zeDriverGet(&count, nullptr);

  if (result) {
    throw std::runtime_error("zeDriverGet failed: " + to_string(result));
  }
  return count;
}

std::vector<ze_driver_handle_t> get_all_driver_handles() {
  ze_result_t result = ZE_RESULT_SUCCESS;
  uint32_t driver_handle_count = get_driver_handle_count();

  std::vector<ze_driver_handle_t> driver_handles(driver_handle_count);

  result = zeDriverGet(&driver_handle_count, driver_handles.data());

  if (result) {
    throw std::runtime_error("zeDriverGet failed: " + to_string(result));
  }
  return driver_handles;
}

// Device
uint32_t get_device_count(ze_driver_handle_t driver) {
  uint32_t count = 0;
  ze_result_t result = zeDeviceGet(driver, &count, nullptr);

  if (result) {
    throw std::runtime_error("zeDeviceGet failed: " + to_string(result));
  }
  return count;
}

std::vector<ze_device_handle_t> get_devices(ze_driver_handle_t driver) {
  ze_result_t result = ZE_RESULT_SUCCESS;

  uint32_t device_count = get_device_count(driver);
  std::vector<ze_device_handle_t> devices(device_count);

  result = zeDeviceGet(driver, &device_count, devices.data());

  if (result) {
    throw std::runtime_error("zeDeviceGet failed: " + to_string(result));
  }
  return devices;
}

ze_device_properties_t get_device_properties(ze_device_handle_t device) {
  ze_device_properties_t properties = {ZE_STRUCTURE_TYPE_DEVICE_PROPERTIES};

  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetProperties(device, &properties));
  return properties;
}

// memory
void* allocate_host_memory(const size_t size, const size_t alignment, const ze_context_handle_t context) {
  ze_host_mem_alloc_desc_t host_desc = {};
  host_desc.stype = ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC;
  host_desc.flags = 0;

  host_desc.pNext = nullptr;

  void* memory = nullptr;
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemAllocHost(context, &host_desc, size, alignment, &memory));
  LEVEL_ZERO_EXPECT_NE(nullptr, memory);

  return memory;
}

void* allocate_device_memory(const size_t size, const size_t alignment, const ze_device_mem_alloc_flags_t flags,
                             const uint32_t ordinal, ze_device_handle_t device_handle, ze_context_handle_t context) {
  void* memory = nullptr;
  ze_device_mem_alloc_desc_t device_desc = {};
  device_desc.stype = ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC;
  device_desc.ordinal = ordinal;
  device_desc.flags = flags;

  device_desc.pNext = nullptr;

  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS,
                       zeMemAllocDevice(context, &device_desc, size, alignment, device_handle, &memory));
  LEVEL_ZERO_EXPECT_NE(nullptr, memory);

  return memory;
}

void* allocate_shared_memory(const size_t size, const size_t alignment, const ze_device_mem_alloc_flags_t dev_flags,
                             const ze_host_mem_alloc_flags_t host_flags, ze_device_handle_t device,
                             ze_context_handle_t context) {
  uint32_t ordinal = 0;
  void* memory = nullptr;
  ze_device_mem_alloc_desc_t device_desc = {};
  device_desc.stype = ZE_STRUCTURE_TYPE_DEVICE_MEM_ALLOC_DESC;
  device_desc.ordinal = ordinal;
  device_desc.flags = dev_flags;

  device_desc.pNext = nullptr;
  ze_host_mem_alloc_desc_t host_desc = {};
  host_desc.stype = ZE_STRUCTURE_TYPE_HOST_MEM_ALLOC_DESC;
  host_desc.flags = host_flags;

  host_desc.pNext = nullptr;
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS,
                       zeMemAllocShared(context, &device_desc, &host_desc, size, alignment, device, &memory));
  LEVEL_ZERO_EXPECT_NE(nullptr, memory);

  return memory;
}

void free_memory(ze_context_handle_t context, void* ptr) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeMemFree(context, ptr));
}

void append_memory_copy(ze_command_list_handle_t cl, void* dstptr, const void* srcptr, size_t size,
                        ze_event_handle_t hSignalEvent, uint32_t num_wait_events, ze_event_handle_t* wait_events) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendMemoryCopy(cl, dstptr, srcptr, size, hSignalEvent,
                                                                        num_wait_events, wait_events));
}

// Module
ze_module_handle_t create_module(ze_context_handle_t context, ze_device_handle_t device, uint8_t* data, size_t bytes,
                                 const ze_module_format_t format, const char* build_flags,
                                 ze_module_build_log_handle_t* p_build_log) {
  ze_module_desc_t module_description = {};
  module_description.stype = ZE_STRUCTURE_TYPE_MODULE_DESC;
  ze_module_handle_t module;
  ze_module_constants_t module_constants = {};
  const std::vector<uint8_t> binary_file(data, data + bytes);

  LEVEL_ZERO_EXPECT_TRUE((format == ZE_MODULE_FORMAT_IL_SPIRV) || (format == ZE_MODULE_FORMAT_NATIVE));

  module_description.pNext = nullptr;
  module_description.format = format;
  module_description.inputSize = static_cast<uint32_t>(binary_file.size());
  module_description.pInputModule = binary_file.data();
  module_description.pBuildFlags = build_flags;
  module_description.pConstants = &module_constants;

  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleCreate(context, device, &module_description, &module, p_build_log));

  return module;
}

void destroy_module(ze_module_handle_t module) { LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeModuleDestroy(module)); }

// Kernel
ze_kernel_handle_t create_function(ze_module_handle_t module, ze_kernel_flags_t flag, std::string func_name) {
  ze_kernel_handle_t kernel;
  ze_kernel_desc_t kernel_description = {};
  kernel_description.stype = ZE_STRUCTURE_TYPE_KERNEL_DESC;

  kernel_description.pNext = nullptr;
  kernel_description.flags = flag;
  kernel_description.pKernelName = func_name.c_str();

  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelCreate(module, &kernel_description, &kernel));
  return kernel;
}

void set_argument_value(ze_kernel_handle_t hFunction, uint32_t argIndex, size_t argSize, const void* pArgValue) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetArgumentValue(hFunction, argIndex, argSize, pArgValue));
}

void append_launch_function(ze_command_list_handle_t hCommandList, ze_kernel_handle_t hFunction,
                            const ze_group_count_t* pLaunchFuncArgs, ze_event_handle_t hSignalEvent,
                            uint32_t numWaitEvents, ze_event_handle_t* phWaitEvents) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendLaunchKernel(hCommandList, hFunction, pLaunchFuncArgs,
                                                                          hSignalEvent, numWaitEvents, phWaitEvents));
}

void destroy_function(ze_kernel_handle_t kernel) { LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelDestroy(kernel)); }

// Command list
ze_command_list_handle_t create_command_list(ze_context_handle_t context, ze_device_handle_t device,
                                             ze_command_list_flags_t flags, uint32_t ordinal) {
  ze_command_list_desc_t descriptor = {};
  descriptor.stype = ZE_STRUCTURE_TYPE_COMMAND_LIST_DESC;

  descriptor.pNext = nullptr;
  descriptor.flags = flags;
  descriptor.commandQueueGroupOrdinal = ordinal;
  ze_command_list_handle_t command_list = nullptr;
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListCreate(context, device, &descriptor, &command_list));
  LEVEL_ZERO_EXPECT_NE(nullptr, command_list);

  return command_list;
}

void close_command_list(ze_command_list_handle_t cl) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListClose(cl));
}

void execute_command_lists(ze_command_queue_handle_t cq, uint32_t numCommandLists,
                           ze_command_list_handle_t* phCommandLists, ze_fence_handle_t hFence) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS,
                       zeCommandQueueExecuteCommandLists(cq, numCommandLists, phCommandLists, hFence));
}

void reset_command_list(ze_command_list_handle_t cl) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListReset(cl));
}

void destroy_command_list(ze_command_list_handle_t cl) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListDestroy(cl));
}

// Command queue
ze_command_queue_handle_t create_command_queue(ze_context_handle_t context, ze_device_handle_t device,
                                               ze_command_queue_flags_t flags, ze_command_queue_mode_t mode,
                                               ze_command_queue_priority_t priority, uint32_t ordinal, uint32_t index) {
  ze_command_queue_desc_t descriptor = {};
  descriptor.stype = ZE_STRUCTURE_TYPE_COMMAND_QUEUE_DESC;

  descriptor.pNext = nullptr;
  descriptor.flags = flags;
  descriptor.mode = mode;
  descriptor.priority = priority;
  ze_device_properties_t properties;
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeDeviceGetProperties(device, &properties));

  descriptor.ordinal = ordinal;
  descriptor.index = index;
  ze_command_queue_handle_t command_queue = nullptr;
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueCreate(context, device, &descriptor, &command_queue));
  LEVEL_ZERO_EXPECT_NE(nullptr, command_queue);

  return command_queue;
}

void synchronize(ze_command_queue_handle_t cq, uint64_t timeout) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueSynchronize(cq, timeout));
}

void destroy_command_queue(ze_command_queue_handle_t cq) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandQueueDestroy(cq));
}

// Event
zeEventPool::zeEventPool() {}

zeEventPool::~zeEventPool() {
  if (event_pool_) {
    ze_result_t result = zeEventPoolDestroy(event_pool_);
    if (ZE_RESULT_SUCCESS != result) {
      std::cout << "Failed to destroy event pool " + to_string(result) << std::endl;
    }
  }
}

void zeEventPool::InitEventPool(ze_context_handle_t context, uint32_t count, ze_event_pool_flags_t flags) {
  LEVEL_ZERO_EXPECT_NE(nullptr, context);
  context_ = context;
  if (event_pool_ == nullptr) {
    ze_event_pool_desc_t descriptor = {};
    descriptor.stype = ZE_STRUCTURE_TYPE_EVENT_POOL_DESC;

    descriptor.pNext = nullptr;
    descriptor.flags = flags;
    descriptor.count = count;

    LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventPoolCreate(context, &descriptor, 0, nullptr, &event_pool_));
    LEVEL_ZERO_EXPECT_NE(nullptr, event_pool_);

    pool_indexes_available_.resize(count, true);
  }
}

void zeEventPool::create_event(ze_event_handle_t* event, ze_event_scope_flags_t signal, ze_event_scope_flags_t wait) {
  // Make sure the event pool is initialized to at least defaults:
  InitEventPool(context_, 32);
  ze_event_desc_t desc = {};
  memset(&desc, 0, sizeof(desc));
  desc.stype = ZE_STRUCTURE_TYPE_EVENT_DESC;
  desc.pNext = nullptr;
  desc.signal = signal;
  desc.wait = wait;
  *event = nullptr;
  desc.index = -1;
  for (uint32_t i = 0; i < pool_indexes_available_.size(); i++) {
    if (pool_indexes_available_[i]) {
      desc.index = i;
      break;
    }
  }
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventCreate(event_pool_, &desc, event));
  LEVEL_ZERO_EXPECT_NE(nullptr, *event);
  handle_to_index_map_[*event] = desc.index;
  pool_indexes_available_[desc.index] = false;
}

void zeEventPool::destroy_event(ze_event_handle_t event) {
  std::map<ze_event_handle_t, uint32_t>::iterator it = handle_to_index_map_.find(event);

  LEVEL_ZERO_EXPECT_NE(it, handle_to_index_map_.end());
  pool_indexes_available_[(*it).second] = true;
  handle_to_index_map_.erase(it);
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeEventDestroy(event));
}

void append_barrier(ze_command_list_handle_t cl, ze_event_handle_t hSignalEvent, uint32_t numWaitEvents,
                    ze_event_handle_t* phWaitEvents) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeCommandListAppendBarrier(cl, hSignalEvent, numWaitEvents, phWaitEvents));
}

// Group
void set_group_size(ze_kernel_handle_t hFunction, uint32_t groupSizeX, uint32_t groupSizeY, uint32_t groupSizeZ) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSetGroupSize(hFunction, groupSizeX, groupSizeY, groupSizeZ));
}

void suggest_group_size(ze_kernel_handle_t hFunction, uint32_t globalSizeX, uint32_t globalSizeY, uint32_t globalSizeZ,
                        uint32_t* groupSizeX, uint32_t* groupSizeY, uint32_t* groupSizeZ) {
  LEVEL_ZERO_EXPECT_EQ(ZE_RESULT_SUCCESS, zeKernelSuggestGroupSize(hFunction, globalSizeX, globalSizeY, globalSizeZ,
                                                                   groupSizeX, groupSizeY, groupSizeZ));
}

// Helper
std::vector<uint8_t> load_binary_file(const std::string& file_path) {
  std::ifstream stream(file_path, std::ios::in | std::ios::binary);

  std::vector<uint8_t> binary_file;
  if (!stream.good()) {
    std::cout << "Failed to load binary file: " << file_path << " error " << strerror(errno) << std::endl;
    return binary_file;
  }

  size_t length = 0;
  stream.seekg(0, stream.end);
  length = static_cast<size_t>(stream.tellg());
  stream.seekg(0, stream.beg);
  std::cout << "Binary file length: " << length << std::endl;

  binary_file.resize(length);
  stream.read(reinterpret_cast<char*>(binary_file.data()), length);
  std::cout << "Binary file loaded" << std::endl;

  return binary_file;
}

std::string to_string(const ze_result_t result) {
  LEVEL_ZERO_ERRSTR(ZE_RESULT_SUCCESS);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_NOT_READY);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNINITIALIZED);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_DEVICE_LOST);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_ARGUMENT);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_OUT_OF_HOST_MEMORY);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_OUT_OF_DEVICE_MEMORY);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_MODULE_BUILD_FAILURE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INSUFFICIENT_PERMISSIONS);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_NOT_AVAILABLE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNSUPPORTED_VERSION);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNSUPPORTED_FEATURE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_NULL_HANDLE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_HANDLE_OBJECT_IN_USE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_NULL_POINTER);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_SIZE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNSUPPORTED_SIZE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNSUPPORTED_ALIGNMENT);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_SYNCHRONIZATION_OBJECT);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_ENUMERATION);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNSUPPORTED_ENUMERATION);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNSUPPORTED_IMAGE_FORMAT);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_NATIVE_BINARY);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_GLOBAL_NAME);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_KERNEL_NAME);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_FUNCTION_NAME);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_GROUP_SIZE_DIMENSION);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_GLOBAL_WIDTH_DIMENSION);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_INDEX);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_KERNEL_ARGUMENT_SIZE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_KERNEL_ATTRIBUTE_VALUE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_INVALID_COMMAND_LIST_TYPE);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_OVERLAPPING_REGIONS);
  LEVEL_ZERO_ERRSTR(ZE_RESULT_ERROR_UNKNOWN);
  throw std::runtime_error("Unknown ze_result_t value: " + std::to_string(static_cast<int>(result)));
}

}  // namespace lzu

// Copyright 2020 Intel Corporation
#include "pmlc/rt/level_zero/level_zero_device.h"

#include "pmlc/rt/jit_executable.h"
#include "pmlc/util/logging.h"

namespace pmlc::rt::level_zero {

LevelZeroQueue::LevelZeroQueue(const ze_context_handle_t& context, const ze_device_handle_t& device,
                               ze_command_queue_group_properties_t properties)
    : properties(properties) {
  queue = lzu::create_command_queue(context, device, /*flags*/ 0, ZE_COMMAND_QUEUE_MODE_DEFAULT,
                                    ZE_COMMAND_QUEUE_PRIORITY_NORMAL,
                                    /*ordinal*/ 0, /*index*/ 0);
  list = lzu::create_command_list(context, device, /*flags*/ 0, /*ordinal*/ 0);
}

LevelZeroQueue::~LevelZeroQueue() {
  lzu::destroy_command_list(list);
  lzu::destroy_command_queue(queue);
}

LevelZeroQueueUser::LevelZeroQueueUser() : LevelZeroQueueUser(nullptr, nullptr) {}

LevelZeroQueueUser::LevelZeroQueueUser(LevelZeroQueueGuard* guard, LevelZeroQueue* queue)
    : guard(guard), queue(queue) {}

LevelZeroQueueUser::LevelZeroQueueUser(LevelZeroQueueUser&& move) : guard(move.guard), queue(move.queue) {
  move.guard = nullptr;
  move.queue = nullptr;
}

LevelZeroQueueUser::~LevelZeroQueueUser() {
  if (guard != nullptr) guard->used.store(false, std::memory_order_relaxed);
}

bool LevelZeroQueueGuard::isUsed() { return used.load(std::memory_order_relaxed); }

LevelZeroQueueUser LevelZeroQueueGuard::use() {
  bool expected = false;
  bool desired = true;
  used.compare_exchange_strong(expected, desired, std::memory_order_relaxed, std::memory_order_relaxed);
  // Currently in use - return null user.
  if (expected) return LevelZeroQueueUser();
  return LevelZeroQueueUser(this, queue);
}

LevelZeroDevice::LevelZeroDevice(ze_driver_handle_t driver, ze_device_handle_t device)
    : driver(driver), device(device) {
  context = lzu::get_context(driver);
  IVLOG(1, "Instantiating LevelZero device: " << lzu::get_device_properties(device).name);
  queues.clear();
}

LevelZeroDevice::~LevelZeroDevice() {
  clearQueues();
  // When we use multiple devices for one driver, the context shall be released
  // carefully.
  clearQueues();
  lzu::destroy_context(context);
}

std::unique_ptr<Executable> LevelZeroDevice::compile(const std::shared_ptr<pmlc::compiler::Program>& program) {
  return makeJitExecutable(program, shared_from_this(), mlir::ArrayRef<void*>{this});
}

LevelZeroQueueUser LevelZeroDevice::getQueue(ze_command_queue_group_properties_t properties) {
  // Lock modification of queues vector.
  std::lock_guard<std::mutex> lock(queuesMutex);
  for (std::unique_ptr<LevelZeroQueueGuard>& guard : queues) {
    // TODO: Open check when other kind of queues are added.
    // All queues are with fixed type now.
    if (/*!(guard->getLevelZeroProperties() & properties) ||*/ guard->isUsed()) {
      continue;
    }
    LevelZeroQueueUser user = guard->use();
    if (user) return user;
  }
  // Because queues is locked and not visible to other threads yet it
  // is safe to assume that new guard will return non-empty LevelZeroQueueUser.
  LevelZeroQueue* newQueue = new LevelZeroQueue(context, device, properties);
  queues.emplace_back(std::make_unique<LevelZeroQueueGuard>(newQueue));
  return queues.back()->use();
}

void LevelZeroDevice::clearQueues() {
  for (std::unique_ptr<LevelZeroQueueGuard>& guard : queues) {
    guard.reset();
  }
  queues.clear();
}

}  // namespace pmlc::rt::level_zero

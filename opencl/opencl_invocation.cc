// Copyright 2020 Intel Corporation
#include "opencl_invocation.h"

#include <algorithm>
#include <chrono>
#include <string>
#include <utility>

#include "pmlc/util/logging.h"

namespace pmlc::rt::level_zero {

ze_event_handle_t
LevelZeroMemory::enqueueRead(ze_command_list_handle_t list, void *dst,
                          const std::vector<ze_event_handle_t> &dependencies) {
  ze_event_handle_t result;
  //queue.enqueueReadBuffer(buffer, /*blocking=*/false, /*offset=*/0, bytes, dst,
  //                        &dependencies, &result);
  lzt::append_memory_copy(list, dst, buffer, bytes, result, dependencies.size(), denepdencies);
  return result;
}

ze_event_handle_t
LevelZeroMemory::enqueueWrite(ze_command_list_handle_t list, void *src,
                           const std::vector<ze_event_handle_t> &dependencies) {
  ze_event_handle_t result;
  //queue.enqueueWriteBuffer(buffer, /*blocking=*/false, /*offset=*/0, bytes, src,
  //                         &dependencies, &result);
  lzt::append_memory_copy(list, buffer, src, bytes, result, dependencies.size(), denepdencies);
  return result;
}

LevelZeroKernel::LevelZeroKernel(ze_module_handle_t module, std::string name)
    : name(std::move(name)) {
        kernel = lzt::create_function(module, name);
}

void LevelZeroKernel::addDependency(ze_event_handle_t event) {
  dependencies.push_back(event);
}

void LevelZeroKernel::setArg(unsigned idx, LevelZeroMemory *memory) {
  //kernel.setArg(idx, memory->getBuffer());
  int* buf = memory->getBuffer();
  lzt::set_argument_value(kernel, idx, sizeof(buf), &buf);
}

ze_event_handle_t LevelZeroKernel::enqueue(ze_command_list_handle_t list, ze_group_count_t gws,
                                ze_group_count_t lws) {
  ze_event_handle_t result;
  //queue.enqueueNDRangeKernel(kernel, /*offset=*/cl::NDRange(), gws, lws,
  //                           &dependencies, &result);
  lzt::append_launch_function(list, kernel, &gws, result, dependencies.size(), &dependencies)
  return result;
}

LevelZeroEvent::LevelZeroEvent(ze_event_handle_t event, LevelZeroActionKind kind,
                         std::string name)
    : event(event), kind(kind), name(std::move(name)) {}

void LevelZeroEvent::wait(const std::vector<ze_event_handle_t> &events) {
  //std::vector<ze> oclEvents;
  //std::transform(events.begin(), events.end(), std::back_inserter(oclEvents),
  //               [](const LevelZeroEvent *event) { return event->getEvent(); });
  //cl::Event::waitForEvents(oclEvents);
  for(auto e : events) {
      zeEventHostSynchronize(e, UINT64_MAX);
  }
}

LevelZeroInvocation::LevelZeroInvocation(LevelZeroDevice *device)
    : device{device->shared_from_this()} {
        ze_command_queue_group_properties_t p;
        p.flags = ZE_COMMAND_QUEUE_GROUP_PROPERTY_FLAG_COMPUTE;
        queueUser = deviec->getQueue(p);
}

LevelZeroInvocation::~LevelZeroInvocation() {
  // Need to explicitly wait for all operations to avoid unfinished events
  // when gathering profiling information.
  finish();
#if 0
  // Gather profiling information.
  using std::chrono::nanoseconds;
  using fp_milliseconds =
      std::chrono::duration<double, std::chrono::milliseconds::period>;
  // Calculate total time as difference between earliest enqueue
  // and latest execution end.
  cl_ulong allQueued = static_cast<cl_ulong>(-1);
  cl_ulong allEnd = 0;
  nanoseconds totalExecuteTime{0};
  nanoseconds kernelExecuteTime{0};
  nanoseconds memoryExecuteTime{0};
  unsigned kernelsCnt = 0;
  unsigned memoryCnt = 0;
  for (std::unique_ptr<LevelZeroEvent> &event : events) {
    cl::Event oclEvent = event->getEvent();
    cl_ulong queued = oclEvent.getProfilingInfo<CL_PROFILING_COMMAND_QUEUED>();
    cl_ulong start = oclEvent.getProfilingInfo<CL_PROFILING_COMMAND_START>();
    cl_ulong end = oclEvent.getProfilingInfo<CL_PROFILING_COMMAND_END>();

    allQueued = std::min(allQueued, queued);
    allEnd = std::max(allEnd, end);

    nanoseconds executeTime{end - start};
    totalExecuteTime += executeTime;

    if (event->getKind() == LevelZeroActionKind::Kernel) {
      kernelExecuteTime += executeTime;
      kernelsCnt += 1;
      IVLOG(2, "  Kernel '" << event->getName() << "' execute time: "
                            << fp_milliseconds(executeTime).count() << "ms");
    } else if (event->getKind() == LevelZeroActionKind::Read ||
               event->getKind() == LevelZeroActionKind::Write) {
      memoryExecuteTime += executeTime;
      memoryCnt += 1;
      IVLOG(2, "  Memory " << event->getName() << " execute time: "
                           << fp_milliseconds(executeTime).count() << "ms");
    }
  }
  nanoseconds totalTime{allEnd - allQueued};
  IVLOG(1, "Total LevelZero time: " << fp_milliseconds(totalTime).count() << "ms");
  IVLOG(1, "Total LevelZero execute time: "
               << fp_milliseconds(totalExecuteTime).count() << "ms");
  IVLOG(1, "Total LevelZero kernels: " << kernelsCnt);
  IVLOG(1, "Total LevelZero kernel execute time: "
               << fp_milliseconds(kernelExecuteTime).count() << "ms");
  IVLOG(1, "Total LevelZero memory transfers: " << memoryCnt);
  IVLOG(1, "Total LevelZero memory transfer time: "
               << fp_milliseconds(memoryExecuteTime).count() << "ms");

  device->execTimeInMS = fp_milliseconds(totalExecuteTime).count();
#endif
}

LevelZeroMemory *LevelZeroInvocation::allocateMemory(size_t bytes) {
  //cl::Buffer buffer(device->getOclContext(), CL_MEM_READ_WRITE, bytes, nullptr);
  void* buffer = lzt::allocate_shared_memory(bytes); 
  return new LevelZeroMemory(buffer, bytes);
}

void LevelZeroInvocation::deallocateMemory(LevelZeroMemory *memory) { delete memory; }

LevelZeroEvent *
LevelZeroInvocation::enqueueRead(LevelZeroMemory *src, void *dst,
                              const std::vector<LevelZeroEvent *> &deps) {
  std::vector<ze_event_handle_t> dependencies;
  std::transform(deps.begin(), deps.end(), std::back_inserter(dependencies),
                 [](const LevelZeroEvent *event) { return event->getEvent(); });
  ze_event_handle_t event =
      src->enqueueRead(queueUser.getLevelZeroList(), dst, dependencies);
  return wrapEvent(event, LevelZeroActionKind::Read, "read");
}

LevelZeroEvent *
LevelZeroInvocation::enqueueWrite(LevelZeroMemory *dst, void *src,
                               const std::vector<LevelZeroEvent *> &deps) {
  std::vector<ze_event_handle_t> dependencies;
  std::transform(deps.begin(), deps.end(), std::back_inserter(dependencies),
                 [](const LevelZeroEvent *event) { return event->getEvent(); });
  ze_event_handle_t event =
      dst->enqueueWrite(queueUser.getLevelZeroList(), src, dependencies);
  return wrapEvent(event, LevelZeroActionKind::Write, "write");
}

LevelZeroKernel *LevelZeroInvocation::createKernelFromIL(char *data, size_t bytes,
                                                   const char *name) {
  std::vector<char> source(data, data + bytes);
  ze_module_handle_t program(lzt::get_default_context(), source, /*build=*/true);
  return new LevelZeroKernel(program, name);
}

LevelZeroEvent *LevelZeroInvocation::enqueueKernel(LevelZeroKernel *kernel,
                                             ze_group_count_t gws, ze_group_count_t lws) {
  ze_event_handle_t event = kernel->enqueue(queueUser.getLevelZeroList(), gws, lws);
  LevelZeroEvent *result =
      wrapEvent(event, LevelZeroActionKind::Kernel, kernel->getName());
  delete kernel;
  return result;
}

LevelZeroEvent *
LevelZeroInvocation::enqueueBarrier(const std::vector<LevelZeroEvent *> &deps) {
  ze_event_handle_t result;
  std::vector<ze_event_handle_t> dependencies;
  std::transform(deps.begin(), deps.end(), std::back_inserter(dependencies),
                 [](const LevelZeroEvent *event) { return event->getEvent(); });
  lzt::append_barrier(queueUser.getLevelZeroList(), result, dependencies.size(), &dependencies);

  return wrapEvent(result, LevelZeroActionKind::Barrier, "barrier");
}

void LevelZeroInvocation::flush() { 
    //queueUser.getOclQueue().flush();
    ze_command_list_handle_t command_list = queueUser.getLevelZeroList();
    ze_command_queue_handle_t command_queue = queueUser.getLevelZeroQueue();
    lzt::close_command_list(command_list);
    lzt::execute_command_lists(command_queue, 1, &command_list, nullptr);
    lzt::synchronize(command_queue, UINT64_MAX);
}

void LevelZeroInvocation::finish() { lzt::synchronize(queueUser.getLevelZeroQueue(), UINT64_MAX); }

LevelZeroEvent *LevelZeroInvocation::wrapEvent(ze_event_handle_t event, LevelZeroActionKind kind,
                                         std::string name) {
  events.emplace_back(
      std::make_unique<LevelZeroEvent>(event, kind, std::move(name)));
  return events.back().get();
}

} // namespace pmlc::rt::level_zero

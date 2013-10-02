// Copyright 2013 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lmctfy/resources/monitoring_resource_handler.h"

#include <vector>

#include "util/errors.h"
#include "util/task/codes.pb.h"

using ::std::vector;
using ::util::StatusOr;

namespace containers {
namespace lmctfy {

StatusOr<MonitoringResourceHandlerFactory *>
MonitoringResourceHandlerFactory::New(
    CgroupFactory *cgroup_factory, const KernelApi *kernel,
    EventFdNotifications *eventfd_notifications) {
  // Perf hierarchy must be mounted.
  if (!cgroup_factory->IsMounted(PerfControllerFactory::HierarchyType())) {
    return ::util::Status(
        ::util::error::NOT_FOUND,
        "Monitoring resource depends on the perf cgroup hierarchy");
  }

  // Create perf controller.
  bool owns_perf =
      cgroup_factory->OwnsCgroup(PerfControllerFactory::HierarchyType());
  PerfControllerFactory *perf_controller = new PerfControllerFactory(
      cgroup_factory, owns_perf, kernel, eventfd_notifications);

  return new MonitoringResourceHandlerFactory(perf_controller, cgroup_factory,
                                              kernel);
}

MonitoringResourceHandlerFactory::MonitoringResourceHandlerFactory(
    const PerfControllerFactory *perf_controller_factory,
    CgroupFactory *cgroup_factory, const KernelApi *kernel)
    : CgroupResourceHandlerFactory(RESOURCE_MONITORING, cgroup_factory, kernel),
      perf_controller_factory_(perf_controller_factory) {}

StatusOr<ResourceHandler *> MonitoringResourceHandlerFactory::
    GetResourceHandler(const string &container_name) const {
  PerfController *controller;
  RETURN_IF_ERROR(perf_controller_factory_->Get(container_name), &controller);
  return new MonitoringResourceHandler(container_name, kernel_, controller);
}

StatusOr<ResourceHandler *> MonitoringResourceHandlerFactory::
    CreateResourceHandler(const string &container_name,
                          const ContainerSpec &spec) const {
  PerfController *controller;
  RETURN_IF_ERROR(perf_controller_factory_->Create(container_name),
                  &controller);
  return new MonitoringResourceHandler(container_name, kernel_, controller);
}

MonitoringResourceHandler::MonitoringResourceHandler(
    const string &container_name, const KernelApi *kernel,
    PerfController *perf_controller)
    : CgroupResourceHandler(container_name, RESOURCE_MONITORING, kernel,
                            vector<CgroupController *>({perf_controller})) {}

util::Status MonitoringResourceHandler::Update(const ContainerSpec &spec,
                                         Container::UpdatePolicy policy) {
  return util::Status::OK;
}

util::Status MonitoringResourceHandler::Stats(Container::StatsType type,
                                              ContainerStats *output) const {
  return util::Status::OK;
}

util::Status MonitoringResourceHandler::Spec(ContainerSpec *spec) const {
  return util::Status::OK;
}

StatusOr<Container::NotificationId>
    MonitoringResourceHandler::RegisterNotification(
        const EventSpec &spec, Callback1< ::util::Status> *callback) {
  ::std::unique_ptr<Callback1< ::util::Status>> callback_deleter(callback);
  return  ::util::Status(::util::error::NOT_FOUND, "No handled event found");
}

}  // namespace lmctfy
}  // namespace containers

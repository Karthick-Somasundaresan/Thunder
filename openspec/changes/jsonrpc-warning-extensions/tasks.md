## 1. Warning Category Classes

- [x] 1.1 Add `ChannelQueueBacklog` warning category class to `Source/core/WarningReportingCategories.h` with `DefaultReportBound=20`, `DefaultWarningBound=50` (note: placed in core/ due to layer discipline - Channel.h is in plugins/ which cannot include from Thunder/)
- [ ] 1.2 Add `NotificationDispatchDuration` warning category class to `Source/Thunder/WarningReportingCategories.h` with `Analyze()` capturing callsign and event name, `DefaultReportBound=50ms`, `DefaultWarningBound=100ms`
- [ ] 1.3 Add `EventSubscriberSaturation` warning category class to `Source/Thunder/WarningReportingCategories.h` with `Analyze()` capturing event name, `DefaultReportBound=20`, `DefaultWarningBound=50`

## 2. Channel Queue Backlog Instrumentation

- [x] 2.1 Add `REPORT_OUTOFBOUNDS_WARNING` in `Channel::Submit(const string&)` after `_sendQueue.emplace_back()` in `Source/plugins/Channel.h`
- [x] 2.2 Add `REPORT_OUTOFBOUNDS_WARNING` in `Channel::Submit(const Core::ProxyType<Core::JSON::IElement>&)` after `_sendQueue.emplace_back()` in `Source/plugins/Channel.h`
- [x] 2.3 Ensure both instrumentation points are guarded by `#ifdef __CORE_WARNING_REPORTING__`

## 3. Notification Dispatch Timing Instrumentation

- [ ] 3.1 Wrap `Server::Notification()` body in `REPORT_DURATION_WARNING` macro in `Source/Thunder/PluginServer.cpp`
- [ ] 3.2 Pass callsign and event name to the `NotificationDispatchDuration::Analyze()` method

## 4. Event Subscriber Saturation Instrumentation

- [ ] 4.1 Add subscriber count check in `JSONRPC::InternalNotify()` before iterating observers in `Source/plugins/JSONRPC.h`
- [ ] 4.2 Call `REPORT_OUTOFBOUNDS_WARNING` with `EventSubscriberSaturation` when `_observers[event].size()` exceeds threshold
- [ ] 4.3 Ensure instrumentation is guarded by `#ifdef __CORE_WARNING_REPORTING__`

## 5. Verification

- [ ] 5.1 Build Thunder with `__CORE_WARNING_REPORTING__` enabled and verify no compile errors
- [ ] 5.2 Build Thunder without `__CORE_WARNING_REPORTING__` and verify no compile errors (zero overhead)

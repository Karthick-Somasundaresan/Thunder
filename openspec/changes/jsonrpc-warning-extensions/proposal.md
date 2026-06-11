## Why

The existing JSONRPC warning infrastructure (commit c4be93b) only captures queue wait time and execution duration. However, notification dispatch paths and per-channel output queues remain invisible, making it difficult to diagnose slow event delivery, backpressure, and subscriber saturation scenarios that cause client timeouts or missed events.

## What Changes

- **New Warning: `ChannelQueueBacklog`** — Reports when a Channel's outgoing message queue (`_sendQueue`) exceeds a threshold, indicating the client is not consuming messages fast enough.
- **New Warning: `NotificationDispatchDuration`** — Measures the time taken to broadcast a notification through the `Service::Notify` → `Server::Notification` → `JEvents::ForwardMessage` path.
- **New Warning: `EventSubscriberSaturation`** — Reports when a notification is sent to a large number of subscribers (indicating potential broadcast storms or slow delivery).

## Capabilities

### New Capabilities
- `channel-queue-backlog`: Warning category for per-channel outgoing message queue depth monitoring
- `notification-dispatch-timing`: Warning category for measuring notification broadcast latency
- `event-subscriber-saturation`: Warning category for detecting high subscriber counts during notification dispatch

### Modified Capabilities
<!-- No existing spec-level behavior changes required -->

## Impact

- **Files Modified**:
  - [Source/core/WarningReportingCategories.h](Source/core/WarningReportingCategories.h) — `ChannelQueueBacklog` warning category class (placed in core/ due to layer discipline)
  - [Source/Thunder/WarningReportingCategories.h](Source/Thunder/WarningReportingCategories.h) — `NotificationDispatchDuration` and `EventSubscriberSaturation` warning category classes
  - [Source/plugins/Channel.h](Source/plugins/Channel.h) — `REPORT_OUTOFBOUNDS_WARNING` in `Submit()` methods
  - [Source/Thunder/PluginServer.cpp](Source/Thunder/PluginServer.cpp) — `REPORT_DURATION_WARNING` in `Server::Notification()`
  - [Source/plugins/JSONRPC.h](Source/plugins/JSONRPC.h) — subscriber count check in `InternalNotify()`
- **APIs**: No external API changes; warning reporting is internal infrastructure
- **Dependencies**: Relies on existing `__CORE_WARNING_REPORTING__` compile flag infrastructure
- **Systems**: Warning output flows through the standard WarningReporting syslog/tracing subsystem

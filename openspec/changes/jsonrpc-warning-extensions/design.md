## Context

Thunder's warning reporting infrastructure already provides visibility into JSONRPC queue wait time (`JSONRPCQueueWait`), worker pool saturation (`WorkerPoolSaturation`), and message invocation duration (`TooLongInvokeMessage`). However, the notification broadcast path and per-channel output queues remain opaque.

Current state:
- Channel's `_sendQueue` grows silently when clients are slow to consume
- Notification dispatch through `Service::Notify` → `Server::Notification` → `JEvents::ForwardMessage` has no timing instrumentation
- High subscriber counts can cause broadcast storms without visibility

Constraints:
- Must use existing `__CORE_WARNING_REPORTING__` compile-time flag
- Must follow existing warning category patterns (`REPORT_OUTOFBOUNDS_WARNING`, `REPORT_DURATION_WARNING`)
- Must not impact normal-path performance when reporting is disabled

## Goals / Non-Goals

**Goals:**
- Provide visibility into per-channel output queue backpressure
- Measure notification broadcast latency end-to-end
- Detect subscriber saturation scenarios that may cause slow delivery

**Non-Goals:**
- Per-subscriber delivery timing (would require significant refactoring)
- Automatic backpressure/throttling (this is observation only)
- Changing notification delivery semantics

## Decisions

### Decision 1: Warning category class placement

**Choice**: Place `NotificationDispatchDuration` and `EventSubscriberSaturation` in `Source/Thunder/WarningReportingCategories.h`. Place `ChannelQueueBacklog` in `Source/core/WarningReportingCategories.h`.

**Rationale**: Layer include discipline requires that `plugins/` cannot include from `Thunder/`. Since `Channel.h` is in `Source/plugins/`, the `ChannelQueueBacklog` category must be accessible from core/. The other two categories are used in Thunder/-layer code and belong there.

**Alternative considered**: Instrumenting in `Server::Channel` (the derived class in Thunder/) — rejected because `Submit()` is not virtual in the base class and would require architectural changes.

### Decision 2: Channel queue measurement point

**Choice**: Instrument `Channel::Submit()` methods to report queue size after each enqueue

**Rationale**: This is the single ingress point for all outgoing messages. Measuring here captures all message types (JSON, text, JSONRPC responses).

**Alternative considered**: Instrumenting `Serialize()` — rejected because it would only capture when messages are actually being sent, missing the accumulation phase.

### Decision 3: Notification timing scope

**Choice**: Wrap the entire `Server::Notification()` method body with `REPORT_DURATION_WARNING`

**Rationale**: This captures the full broadcast path including the `JEvents::Event::ForwardMessage` call which iterates through all subscribers.

**Alternative considered**: Instrumenting each individual subscriber callback — rejected as too invasive and would change the notification semantics.

### Decision 4: Subscriber count threshold approach

**Choice**: Report in `JSONRPC::InternalNotify()` when `_observers[event].size()` exceeds threshold

**Rationale**: This is where the event-to-subscriber mapping lives. The observer list size directly indicates broadcast fanout.

**Alternative considered**: Counting in `Server::Notification()` — rejected because the subscriber registry is in the JSONRPC layer, not the server layer.

## Risks / Trade-offs

**[Risk: Performance overhead on hot path]** → Mitigated by `#ifdef __CORE_WARNING_REPORTING__` guards. When disabled, zero overhead. When enabled, overhead is a timestamp read and integer comparison.

**[Risk: Warning spam during legitimate high load]** → Mitigated by appropriate default thresholds and the existing report/warning two-tier system that allows filtering.

**[Trade-off: Queue size vs. queue wait time]** → We chose to measure queue SIZE (depth) rather than wait time because:
1. Size is cheaper to compute (no timestamp storage per message)
2. Size correlates with memory pressure
3. Wait time is already covered by `JSONRPCQueueWait` for JSONRPC messages

**[Risk: Subscriber count changes during iteration]** → Accepted as low-impact. The count is a snapshot at notification time; slight inaccuracy is acceptable for monitoring purposes.

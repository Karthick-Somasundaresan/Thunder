## ADDED Requirements

### Requirement: Channel queue depth monitoring
The warning reporting system SHALL monitor the depth of each Channel's outgoing message queue (`_sendQueue`) and emit a warning when the queue size exceeds the configured threshold.

#### Scenario: Queue depth exceeds report threshold
- **WHEN** a message is submitted to a Channel via `Submit()` and the resulting queue size exceeds `DefaultReportBound`
- **THEN** the system SHALL emit an info-level warning containing the channel ID and current queue depth

#### Scenario: Queue depth exceeds warning threshold
- **WHEN** a message is submitted to a Channel via `Submit()` and the resulting queue size exceeds `DefaultWarningBound`
- **THEN** the system SHALL emit a warning-level message containing the channel ID and current queue depth

#### Scenario: Queue depth below threshold
- **WHEN** a message is submitted to a Channel via `Submit()` and the resulting queue size is below `DefaultReportBound`
- **THEN** the system SHALL NOT emit any warning

### Requirement: ChannelQueueBacklog warning category class
The system SHALL provide a `ChannelQueueBacklog` warning category class in `Source/Thunder/WarningReportingCategories.h` that follows the standard warning category pattern.

#### Scenario: Class structure compliance
- **WHEN** the `ChannelQueueBacklog` class is defined
- **THEN** the class SHALL implement `Serialize()`, `Deserialize()`, and `ToString()` methods
- **THEN** the class SHALL define `DefaultWarningBound` as 50 (messages)
- **THEN** the class SHALL define `DefaultReportBound` as 20 (messages)

#### Scenario: Warning output format
- **WHEN** `ToString()` is called on `ChannelQueueBacklog`
- **THEN** the output SHALL include the channel ID and the actual queue depth value

## ADDED Requirements

### Requirement: Event subscriber count monitoring
The warning reporting system SHALL monitor the number of subscribers receiving a notification and emit a warning when the subscriber count exceeds the configured threshold.

#### Scenario: Subscriber count exceeds report threshold
- **WHEN** a notification is dispatched via `JSONRPC::InternalNotify()` and the number of registered observers for the event exceeds `DefaultReportBound`
- **THEN** the system SHALL emit an info-level warning containing the event name and subscriber count

#### Scenario: Subscriber count exceeds warning threshold
- **WHEN** a notification is dispatched via `JSONRPC::InternalNotify()` and the number of registered observers for the event exceeds `DefaultWarningBound`
- **THEN** the system SHALL emit a warning-level message containing the event name and subscriber count

#### Scenario: Subscriber count within normal range
- **WHEN** a notification is dispatched via `JSONRPC::InternalNotify()` and the number of registered observers is below `DefaultReportBound`
- **THEN** the system SHALL NOT emit any warning

### Requirement: EventSubscriberSaturation warning category class
The system SHALL provide an `EventSubscriberSaturation` warning category class in `Source/Thunder/WarningReportingCategories.h` that follows the standard warning category pattern.

#### Scenario: Class structure compliance
- **WHEN** the `EventSubscriberSaturation` class is defined
- **THEN** the class SHALL implement `Analyze()`, `Serialize()`, `Deserialize()`, and `ToString()` methods
- **THEN** the class SHALL capture the event name for diagnostic output
- **THEN** the class SHALL define `DefaultWarningBound` as 50 (subscribers)
- **THEN** the class SHALL define `DefaultReportBound` as 20 (subscribers)

#### Scenario: Warning output format
- **WHEN** `ToString()` is called on `EventSubscriberSaturation`
- **THEN** the output SHALL include the event name, actual subscriber count, and threshold value

### Requirement: Conditional compilation guard
All subscriber saturation monitoring code SHALL be guarded by `#ifdef __CORE_WARNING_REPORTING__` to ensure zero overhead when warning reporting is disabled.

#### Scenario: Warning reporting disabled
- **WHEN** the code is compiled without `__CORE_WARNING_REPORTING__` defined
- **THEN** the subscriber count check and warning emission code SHALL NOT be included in the binary

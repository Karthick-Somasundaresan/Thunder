## ADDED Requirements

### Requirement: Notification dispatch duration monitoring
The warning reporting system SHALL measure the time taken to dispatch a notification through the `Server::Notification()` path and emit a warning when the duration exceeds the configured threshold.

#### Scenario: Dispatch duration exceeds report threshold
- **WHEN** a notification is dispatched via `Server::Notification()` and the total dispatch time exceeds `DefaultReportBound`
- **THEN** the system SHALL emit an info-level warning containing the callsign, event name, and dispatch duration in milliseconds

#### Scenario: Dispatch duration exceeds warning threshold
- **WHEN** a notification is dispatched via `Server::Notification()` and the total dispatch time exceeds `DefaultWarningBound`
- **THEN** the system SHALL emit a warning-level message containing the callsign, event name, and dispatch duration in milliseconds

#### Scenario: Dispatch completes within threshold
- **WHEN** a notification is dispatched via `Server::Notification()` and the total dispatch time is below `DefaultReportBound`
- **THEN** the system SHALL NOT emit any warning

### Requirement: NotificationDispatchDuration warning category class
The system SHALL provide a `NotificationDispatchDuration` warning category class in `Source/Thunder/WarningReportingCategories.h` that follows the standard warning category pattern.

#### Scenario: Class structure compliance
- **WHEN** the `NotificationDispatchDuration` class is defined
- **THEN** the class SHALL implement `Analyze()`, `Serialize()`, `Deserialize()`, and `ToString()` methods
- **THEN** the class SHALL capture the callsign and event name for diagnostic output
- **THEN** the class SHALL define `DefaultWarningBound` as 100 (milliseconds)
- **THEN** the class SHALL define `DefaultReportBound` as 50 (milliseconds)

#### Scenario: Warning output format
- **WHEN** `ToString()` is called on `NotificationDispatchDuration`
- **THEN** the output SHALL include the plugin callsign, event name, actual duration, and threshold value

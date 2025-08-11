# Requirements Document

## Introduction

This document outlines the requirements for a statistics probability and distribution calculator application designed for the Xiaomi Miband 10 running Vela OS. The app will provide users with essential statistical calculation capabilities directly on their wearable device, enabling quick probability calculations and distribution analysis for educational, professional, or personal use.

## Requirements

### Requirement 1

**User Story:** As a user, I want to calculate basic probability distributions, so that I can perform statistical analysis on my wearable device.

#### Acceptance Criteria

1. WHEN the user selects a distribution type THEN the system SHALL display the appropriate input fields for that distribution
2. WHEN the user enters valid parameters for a distribution THEN the system SHALL calculate and display the probability density function (PDF) value
3. WHEN the user enters valid parameters for a distribution THEN the system SHALL calculate and display the cumulative distribution function (CDF) value
4. IF the user enters invalid parameters THEN the system SHALL display an error message and prevent calculation
5. WHEN the user requests a calculation THEN the system SHALL complete the calculation within 2 seconds

### Requirement 2

**User Story:** As a user, I want to access comprehensive probability distributions (Normal, Exponential, Geometric, Hypergeometric, Binomial, Negative Binomial, Poisson, Chi-Square, t-distribution, F-distribution), so that I can perform various statistical calculations.

#### Acceptance Criteria

1. WHEN the user opens the app THEN the system SHALL display a categorized list of available distributions including continuous and discrete distributions
2. WHEN the user selects Normal distribution THEN the system SHALL provide input fields for mean and standard deviation
3. WHEN the user selects Exponential distribution THEN the system SHALL provide input field for rate parameter (lambda)
4. WHEN the user selects Geometric distribution THEN the system SHALL provide input field for probability of success
5. WHEN the user selects Hypergeometric distribution THEN the system SHALL provide input fields for population size, success states, and sample size
6. WHEN the user selects Binomial distribution THEN the system SHALL provide input fields for number of trials and probability of success
7. WHEN the user selects Negative Binomial distribution THEN the system SHALL provide input fields for number of successes and probability of success
8. WHEN the user selects Poisson distribution THEN the system SHALL provide input field for lambda (rate parameter)
9. WHEN the user selects Chi-Square distribution THEN the system SHALL provide input field for degrees of freedom
10. WHEN the user selects t-distribution THEN the system SHALL provide input field for degrees of freedom
11. WHEN the user selects F-distribution THEN the system SHALL provide input fields for numerator and denominator degrees of freedom
12. WHEN the user switches between distributions THEN the system SHALL clear previous inputs and display appropriate parameter fields

### Requirement 3

**User Story:** As a user, I want to view calculation results in a clear format, so that I can easily interpret the statistical values.

#### Acceptance Criteria

1. WHEN a calculation is completed THEN the system SHALL display results with appropriate precision (4 decimal places)
2. WHEN displaying results THEN the system SHALL clearly label PDF and CDF values
3. WHEN displaying results THEN the system SHALL show the input parameters used for the calculation
4. IF a result is very small (< 0.0001) THEN the system SHALL display it in scientific notation
5. WHEN the screen is small THEN the system SHALL format results to fit the Miband display constraints

### Requirement 4

**User Story:** As a user, I want to navigate the app using touch gestures, so that I can operate it efficiently on the small wearable screen.

#### Acceptance Criteria

1. WHEN the user swipes left or right THEN the system SHALL navigate between different distribution types
2. WHEN the user taps on an input field THEN the system SHALL activate the numeric input interface
3. WHEN the user performs a long press on the calculate button THEN the system SHALL execute the calculation
4. WHEN the user swipes up THEN the system SHALL scroll through calculation history if available
5. WHEN the user taps the back gesture area THEN the system SHALL return to the previous screen

### Requirement 5

**User Story:** As a user, I want the app to work efficiently on Vela OS, so that it doesn't drain my device battery or cause performance issues.

#### Acceptance Criteria

1. WHEN the app is running THEN the system SHALL consume less than 5% of available CPU resources during idle state
2. WHEN performing calculations THEN the system SHALL complete operations without blocking the UI thread
3. WHEN the app is backgrounded THEN the system SHALL reduce memory usage by at least 50%
4. WHEN the device is in low power mode THEN the system SHALL continue to function with reduced refresh rates
5. WHEN the app starts THEN the system SHALL launch within 3 seconds on the Miband 10 hardware

### Requirement 6

**User Story:** As a user, I want input validation and error handling, so that I can understand when I've entered incorrect values.

#### Acceptance Criteria

1. WHEN the user enters non-numeric values THEN the system SHALL display "Invalid number format" error
2. WHEN the user enters parameters outside valid ranges THEN the system SHALL display specific range error messages
3. WHEN a calculation fails due to mathematical constraints THEN the system SHALL display "Calculation not possible" with explanation
4. WHEN the system encounters an error THEN the system SHALL allow the user to retry without restarting the app
5. WHEN displaying error messages THEN the system SHALL clear them after 3 seconds or when user takes action

### Requirement 7

**User Story:** As a user, I want distributions organized by type (continuous vs discrete), so that I can quickly find the appropriate distribution for my analysis.

#### Acceptance Criteria

1. WHEN the user opens the distribution selector THEN the system SHALL display two categories: "Continuous" and "Discrete"
2. WHEN the user selects "Continuous" THEN the system SHALL show Normal, Exponential, Chi-Square, t-distribution, and F-distribution
3. WHEN the user selects "Discrete" THEN the system SHALL show Geometric, Hypergeometric, Binomial, Negative Binomial, and Poisson
4. WHEN viewing a category THEN the system SHALL display distribution names with brief parameter hints
5. WHEN the user navigates between categories THEN the system SHALL maintain smooth transitions suitable for the small screen

### Requirement 8

**User Story:** As a user, I want to save and recall recent calculations, so that I can reference previous statistical work.

#### Acceptance Criteria

1. WHEN a calculation is completed THEN the system SHALL automatically save it to calculation history
2. WHEN the user accesses history THEN the system SHALL display the last 10 calculations with timestamps
3. WHEN the user selects a historical calculation THEN the system SHALL restore the parameters and results
4. WHEN storage is full THEN the system SHALL remove the oldest calculation to make room for new ones
5. WHEN the app is restarted THEN the system SHALL persist and restore the calculation history
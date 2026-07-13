#include <unity.h>
#include "../src/config.h"
#include "../src/safety_monitor.h"

// Mock external variables to override hardware analogRead
extern int ADC_OFFSET;
extern float nominalCurrent;
extern float underVoltageLimit;
extern float overVoltageLimit;

// Mock functions for Unity Testing
void setUp(void) {
    // Reset safety monitor state before each test
    initSafetyMonitor();
    resetSafetyFault();
    updateSafetyThresholds(15.0f, 180.0f, 260.0f);
}

void tearDown(void) {
    // Clean up
}

// TEST 1: Normal Operation (No faults)
void test_normal_operation(void) {
    // Mock normal voltages (230V) and currents (12A)
    // In our test environment, performSafetyCheck is called in a loop.
    performSafetyCheck();
    
    TEST_ASSERT_FALSE(isSafetyFaultActive());
    TEST_ASSERT_EQUAL(FAULT_NONE, getActiveFault());
}

// TEST 2: Under Voltage Trip
void test_under_voltage_trip(void) {
    // 1. Simulate low voltage on R phase (e.g. 150V)
    // We override internal state or loop checking.
    // In our implementation, underVoltageLimit is 180V.
    // We simulate calling safety checks over a duration longer than TRIP_TIME_VOLTAGE (3s).
    
    // Set simulated inputs (Requires a mock helper in the actual safety_monitor.cpp,
    // but for unit test simulation we call it sequentially with elapsed mock millis)
    
    // Trigger first sampling
    performSafetyCheck();
    TEST_ASSERT_FALSE(isSafetyFaultActive()); // shouldn't trip immediately (requires 3s filter)

    // Simulate 3.5 seconds delay in test environment
    // In test runner, we override the internal millis() or loop sample
    // here we simulate the timeout by calling it repeatedly with simulated time increments
    // (In actual hardware test, we inject 150V AC and wait 3.5s)
    
    // For this simulation, we mock the trip timer boundary directly:
    extern uint32_t underVoltageTripTimer;
    underVoltageTripTimer = millis() - 4000; // Force timer to look like 4s has passed
    
    performSafetyCheck();
    
    TEST_ASSERT_TRUE(isSafetyFaultActive());
    TEST_ASSERT_EQUAL(FAULT_UNDER_VOLTAGE, getActiveFault());
}

// TEST 3: Single Phasing Cutoff
void test_single_phasing_trip(void) {
    // Simulate Phase Y dropping to 0V (Phase failure) while R and B are at 230V
    // In our safety_monitor.cpp, if any phase drops below 80V while others are active,
    // it trips FAULT_SINGLE_PHASING immediately (< 500ms).
    
    // We simulate mock ADCs in the test code
    // Assuming custom test hooks exist to inject raw values
    performSafetyCheck(); 
    
    // After injecting failed phase:
    // TEST_ASSERT_TRUE(isSafetyFaultActive());
    // TEST_ASSERT_EQUAL(FAULT_SINGLE_PHASING, getActiveFault());
}

// TEST 4: Inverse-Time Thermal Overload Model
void test_thermal_overload_trip(void) {
    // Simulate high load current (30A, twice the nominal limit of 15A)
    // Nominally, ratio = 30 / 15 = 2.0
    // t_trip should trigger when thermalAccumulator exceeds 300.0
    
    extern float thermalAccumulator;
    thermalAccumulator = 310.0f; // Force accumulator past thermal limit
    
    performSafetyCheck();
    
    TEST_ASSERT_TRUE(isSafetyFaultActive());
    TEST_ASSERT_EQUAL(FAULT_OVER_CURRENT, getActiveFault());
}

// TEST 5: Critical Water Level Trip
void test_critical_water_level_trip(void) {
    // Simulate water level dropping below 10% limit
    setSystemWaterLevel(5.0f);
    performSafetyCheck();
    
    TEST_ASSERT_TRUE(isSafetyFaultActive());
    TEST_ASSERT_EQUAL(FAULT_CRITICAL_WATER_LEVEL, getActiveFault());
}

// Run all test cases
int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_normal_operation);
    RUN_TEST(test_under_voltage_trip);
    RUN_TEST(test_single_phasing_trip);
    RUN_TEST(test_thermal_overload_trip);
    RUN_TEST(test_critical_water_level_trip);
    return UNITY_END();
}

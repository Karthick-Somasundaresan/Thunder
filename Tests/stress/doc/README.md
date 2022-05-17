# Description
  This test framework will help in Load testing and Stress Testing a given class with minimal implementation from the users.
  
## Load Test
  Load tests help us understand how a system behaves under an expected load. This will try to simulate a general traffic on the class under test (CUT) with some interval between requests. For simulation the framework uses different waveforms to generate traffic
  
## Stress Test
  Stress test is bit different from Load test. Stress tests help you understand the upper limits of a system's capacity using a load beyond the expected maximum.
  In this test there will be minimum of two threads one will continously try to increase the load and the other will try to decrease the load. 
  
# Usage
  The framework expose an interface which the user need to implement. This interface will have APIs to IncreaseLoad and DecreaseLoad. The implementation of these functions should call the APIs which will add load to the class and decrease load to the CUT respectively 

## Directory Structure.
  We have the following directories
  | Directory| Description|
  |----|----|
  |include | Contains the headerfiles related to the framework |
  |lib | Contains the source code for the framework|
  |tests | Place to add new tests |
  | doc | Contains documentation related to the Stress/Load test|

 
## Useful Macros
  Once we create the implementation for StressTestInterface, we can use that implementation to create Load test using the below macros.
  | Macro | Description | Sample Usage |
  |----|----|----|
  |**LOAD_TEST**(<Class_name>, [n number of args to initialize class] )| Used to create a load test with default number of threads(count 3)|**LOAD_TEST**(ProxyPoolStressTest); *//ProxyPoolStressTest in an impl of StressTestInterface*|
  |**LOAD_TEST_WITH_CUSTOM_THREADS**(<No. of Threads>, <Class_Name>, [n number of args to initialize class])| Instead of default 3 threads to create traffic the specified number of threads will be spawned to simulate the traffic |**LOAD_TEST_WITH_CUSTOM_THREADS**(4, ProxyPoolStressTest); *//ProxyPoolStressTest in an impl of StressTestInterface*|
  |**STRESS_TEST**(<Class_name>, [n number of args to initialize class] )| Used to create a stress test with default number of threads(count 2)|**STRESS_TEST**(ProxyPoolStressTest); *//ProxyPoolStressTest in an impl of StressTestInterface*|
  |**STRESS_TEST_WITH_CUSTOM_THREADS**(<No. of Threads>, <Class_Name>, [n number of args to initialize class])| Instead of default 2 threads to create traffic the specified number of threads will be spawned to create the load and unload the class |**STRESS_TEST_WITH_CUSTOM_THREADS**(4, ProxyPoolStressTest); *//ProxyPoolStressTest in an impl of StressTestInterface*|
  |**BEGIN_GROUP(group_name)**| Used to group all the test cases between this tag and the **END_GROUP** tag to create a category|**BEGIN_GROUP**(MyGroup) ... **END_GROUP** |
  

## Adding a new test
  For adding a new test, one has to write an implementation for the StressTestInterface class present in the StressTestCommon.h. That implementation will have the CUT as a member and corresponding APIs should be called to add and reduce load. At the end of the file use the above said macros to create Stress or Load tests.
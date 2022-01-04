/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 Metrological
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __STRESS_TEST_MANAGER_H_
#define __STRESS_TEST_MANAGER_H_
#include <map>
#include "StressTestLoadFactory.h"
#include "StressTestTrafficGenerator.h"
#include "StressTestExecutionStrategies.h"
namespace WPEFramework {
namespace StressTest {

class TestManager: public TestManagerInterface {
  public:
    TestManager() = delete;
    TestManager(const TestManager&) = delete;
    TestManager& operator=(const TestManager&) = delete;
    ~TestManager() {
      while(!_taskExecutionList.empty()) {
        delete _taskExecutionList.front();
        _taskExecutionList.pop_front();
      }
      delete _loadTestObject;
    }
    TestManager(uint32_t duration, uint32_t freq, uint32_t maxMem, string objTypeName): _loadTestObject(nullptr)
                                                                  , _trafficGeneratorFactory(duration, freq, maxMem)
                                                                  , _taskExecutionList()
                                                                  , _cs(0, _trafficGeneratorFactory.getNumberOfTrafficGenerators())
                                                                  , _timerCount(0){

      _loadTestObject = LoadTestClassFactory::getConcreteTestObject(objTypeName, maxMem);
      if(freq > 0) {

        for (const auto iter: _trafficGeneratorFactory.getAllTrafficGenerators()){
          _taskExecutionList.emplace_back(new LinearTimedTaskExecutor(this, _loadTestObject, iter));
        }
      }
      else {
        _taskExecutionList.emplace_back(new OnlyLoadThreadTaskExecutor(this, _loadTestObject, duration));
        _taskExecutionList.emplace_back(new OnlyUnLoadThreadTaskExecutor(this, _loadTestObject, duration));
      }

    }
    void NotifyTestComplete() override;
    void NotifyTestComplete(std::map<uint64_t, int32_t>& report) override;

    void generateReport();
    
    void WaitForTestToComplete();
    void StartTest();
    
  private:
    LoadTestInterface* _loadTestObject;
    TrafficGeneratorFactory _trafficGeneratorFactory;
    std::list<TaskExecutionInterface*> _taskExecutionList;
    Core::CountingSemaphore _cs;
    uint32_t _timerCount;
    std::multimap<uint64_t, uint32_t> _masterReport;
};

} //StressTest
} //WPEFramework
#endif
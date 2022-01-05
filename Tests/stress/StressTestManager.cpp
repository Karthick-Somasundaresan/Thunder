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

#define MODULE_NAME ProxyPoolStressTest
#include "StressTestUtil.h"
#include "StressTestManager.h"
#include "StressTestExecutionStrategies.h"

namespace WPEFramework {
namespace StressTest {

void TestManager::NotifyTestComplete() {
  _cs.Unlock();
  return;
}
void TestManager::NotifyTestComplete(std::map<uint64_t, int32_t>& report) /*override*/ {
  std::cout<<"Test Completed\n";
  for(const auto & elem : report) {
    _masterReport.insert(std::pair<uint64_t, int32_t>(elem.first, elem.second));
  }
  std::cout<<"Timer Completed\n";
  _cs.Unlock();
  return;

}

void TestManager::generateReport() {
  int32_t peak{1};
  int32_t total{0};
  uint32_t noOfRequest = std::count_if(_masterReport.cbegin(), _masterReport.cend(), [](std::pair<uint64_t, int32_t> element){ return element.second > 0? true:false;});
  uint32_t noOfRelease = std::count_if(_masterReport.cbegin(), _masterReport.cend(), [](std::pair<uint64_t, int32_t> element){ return element.second < 0? true:false;});
  auto lambda =  [&peak, &total](std::pair<uint64_t, int32_t> element)mutable{
                        total += element.second;
      if (total > peak)
      {
        peak = total;
      }};
  std::for_each(_masterReport.begin(), _masterReport.end(), lambda);
  std::cout<<"No. of Operation: "<<_masterReport.size()<<"\n";
  std::cout<<"No. of Request: "<< noOfRequest<<"\n";
  std::cout<<"No. of Release: "<< noOfRelease<<"\n";
  std::cout<<"Peak request: "<< peak<<"\n";


  return;
}
    
void TestManager::WaitForTestToComplete() {
  while(_timerCount > 0) {
  _timerCount--;
  _cs.Lock();
  }
  for(const auto& iter: _taskExecutionList) {
    iter->Cleanup();
  }
  generateReport();
  return;
}
void TestManager::StartTest() {
  ASSERT(_loadTestObject != nullptr);
  if(_freq > 0) {

    for (const auto iter: _trafficGeneratorFactory.getAllTrafficGenerators()){
      _taskExecutionList.emplace_back(new LinearTimedTaskExecutor(this, _loadTestObject, iter));
    }
  }
  else {
    _taskExecutionList.emplace_back(new OnlyLoadThreadTaskExecutor(this, _loadTestObject, _duration));
    _taskExecutionList.emplace_back(new OnlyUnLoadThreadTaskExecutor(this, _loadTestObject, _duration));
  }
  for(const auto& iter: _taskExecutionList) {
    iter->ExecuteTest();
    _timerCount++;
  }
  WaitForTestToComplete();
  std::cout<<"All tests completed\n";
}

void TestManager::SetTestObject(LoadTestInterface* loadTestObject) {
  ASSERT(loadTestObject != nullptr);
  _loadTestObject = loadTestObject;
}

} // namespace StressTest

    
} // namespace WPEFramework

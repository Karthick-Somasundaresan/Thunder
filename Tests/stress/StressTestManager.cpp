/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 Metrological
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

#include "StressTestCommon.h"

namespace WPEFramework {
namespace StressTest {

TestManager& TestManager::Instance() {
  static TestManager tm;
  return tm;
}

TestManager::~TestManager(){
  std::cerr<<"TestManager Destructor\n";
}
void TestManager::UnRegisterTest(TestInterface* testInterface) {
  auto result = std::find(std::begin(_listOfTests), std::end(_listOfTests), testInterface);
  if(result != std::end(_listOfTests)){
    _listOfTests.erase(result);
  }
}
void TestManager::RegisterTest(TestInterface* testInterface) {
  std::cout<<"Registering test category: "<<testInterface->GetName()<<'\n';
  testInterface->SetListener(this);
  _listOfTests.push_back(testInterface);
}

void TestManager::PerformTest() {
  std::cout<<"List of test/groups executions registered: "<<_listOfTests.size()<<'\n';
  for(auto iter = _listOfTests.begin(); iter != _listOfTests.end() && IsTestCancelled() != true; iter++) {
      std::cerr<<"Start testing for category: "<<(*iter)->GetName()<<'\n';
      _executionCount++;
      (*iter)->ExecuteTest();
      WaitForCompletion();
  }
  std::cout<<"All Test Completed TestManager\n";

  std::cerr<<"Waiting 1 sec for cooldown\n";
  sleep(1);
}

void TestManager::WaitForCompletion() {
  while(_executionCount > 0) {
      if(_cs.Lock(wakeInterval) == Core::ERROR_NONE) {
        _executionCount--;
      }
  }
}

bool TestManager::IsTestCancelled() const {
  std::lock_guard<std::mutex> lock(_lock);
  std::cerr<<"CANCEL TEST STATE: "<<_cancelTest<<'\n';
  return _cancelTest;
}

void TestManager::CancelTest() {
  std::lock_guard<std::mutex> lock(_lock);
  _cancelTest = true;
}

void TestManager::StopExecution() {

  std::cerr<<"Stopping Execution\n";
  CancelTest();
  std::cerr<<"Calling cancel test on all Registered Test\n";
  for(auto iter = _listOfTests.begin(); iter != _listOfTests.end(); iter++) {

    if((*iter)->GetExecutionState() == ExecutionState::RUNNING){
      std::cerr<<"Calling CancelTest on "<<(*iter)->GetName()<<'\n';
      (*iter)->CancelTest();
    }
  }
}
void TestManager::HandleComplete() {
  std::cerr<<"Received Handle Complete in TestManager\n";
    _cs.Unlock();
}

void TestManager::HandleCancelRequest() {
  _monitorCancelRequestThread.Run();
}

} // namespace StressTest
} // namespace WPEFramework
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
}
void TestManager::UnRegisterTest(AbstractTestInterface* AbstractTestInterface) {
  auto result = std::find(std::begin(_listOfTests), std::end(_listOfTests), AbstractTestInterface);
  if(result != std::end(_listOfTests)){
    _listOfTests.erase(result);
  }
}
void TestManager::RegisterTest(AbstractTestInterface* AbstractTestInterface) {
  std::cout<<"Registering test category: "<<AbstractTestInterface->GetName()<<'\n';
  AbstractTestInterface->SetListener(this);
  _listOfTests.push_back(AbstractTestInterface);
}

void TestManager::PerformTest() {
  std::cout<<"List of test/groups executions registered: "<<_listOfTests.size()<<'\n';
  for(auto iter: _listOfTests) {
    iter->Reset();
  }
  _timeKeeper.Reset();
  _timeKeeper.Start();
  for(auto iter = _listOfTests.begin(); iter != _listOfTests.end() && IsTestCancelled() != true; iter++) {
      std::cerr<<"Start testing : "<<(*iter)->GetName()<<'\n';
      _executionCount++;
      (*iter)->ExecuteTest();
      std::cerr<<"Waiting for completion :"<<(*iter)->GetName()<<'\n';
      WaitForCompletion();
      std::cerr<<"End testing: "<<(*iter)->GetName()<<'\n';
  }
  _timeKeeper.Stop();
  std::cout<<"All Test Completed TestManager\n";
  PrintReport(ReportType::ALL);

  //std::cerr<<"Waiting 1 sec for cooldown\n";
  //sleep(1);
  _performTestThread.Block();
}

void TestManager::WaitForCompletion() {
  while(_executionCount > 0) {
      if(_cs.Lock(wakeInterval) == Core::ERROR_NONE) {
        _executionCount--;
      }
  }
  if(IsTestCancelled()) {
    _cancelWait.Unlock();
  }
}

void TestManager::StartTest() {
  std::cout<<"Enter Start Test\n";
  if(_performTestThread.State() != Core::Thread::RUNNING){
    std::cout<<"Running perform Test thread again\n";
    _performTestThread.Run();
    std::cout<<"Running perform Test thread completed\n";
  } else {
    std::cout<<"Already Test Started\n";
    std::cout<<"Current thread State: "<<_performTestThread.State()<<"\n";
    //_performTestThread.Block();
    _performTestThread.Wait(Core::Thread::SUSPENDED| Core::Thread::BLOCKED|Core::Thread::STOPPED, Core::infinite);
    std::cout<<"After Wait, Current thread State: "<<_performTestThread.State()<<"\n";
    std::cout<<"Old Run Stopped. Starting a new run\n";

    _performTestThread.Run();
    std::cout<<"After new Run\n";
    std::cout<<"After new Run, Current thread State: "<<_performTestThread.State()<<"\n";
  }
}

void TestManager::PrintReport(ReportType type) const {
  
  if(type ==  ReportType::TEST_COUNT ) {
    std::cout<<"No. of tests: "<<_listOfTests.size()<<'\n';
  } else {
    for (auto & iter: _listOfTests) {
      iter->PrintReport(type);
    }
  }
  std::cout<<"\nTotal ElapsedTime: "<<_timeKeeper.GetElapsedTimeStr()<<"\n\n";
}

bool TestManager::IsTestCancelled() const {
  std::lock_guard<std::mutex> lock(_lock);
  return _cancelTest;
}

void TestManager::CancelTest() {
  std::lock_guard<std::mutex> lock(_lock);
  _cancelTest = true;
}

void TestManager::StopExecution() {

  bool waitForExecutionComplete = false;
  std::cerr<<"Stopping Execution\n";
  CancelTest();
  std::cerr<<"Calling cancel test on all Registered Test count:" << _listOfTests.size()<<"\n";
  for(auto iter = _listOfTests.begin(); iter != _listOfTests.end(); iter++) {

    if((*iter)->GetExecutionState() == ExecutionState::RUNNING){
      std::cerr<<"Calling CancelTest on "<<(*iter)->GetName()<<'\n';
      (*iter)->CancelTest();
      waitForExecutionComplete = true;
    }
  }
  if (waitForExecutionComplete)
    _cancelWait.Lock();
  std::cerr<<"All Test Stopped\n";
}
void TestManager::HandleComplete() {
  std::cerr<<"Received Handle Complete in TestManager\n";
    _cs.Unlock();
}

} // namespace StressTest
} // namespace WPEFramework

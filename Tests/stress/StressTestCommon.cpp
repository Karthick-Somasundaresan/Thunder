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
#include "StressTestAdapters.h"

namespace WPEFramework{
namespace StressTest{
void CategoryTest::Register(string categoryName, AbstractTestInterface* testClass) {
    _categoryMap[categoryName].push_back(testClass);
    testClass->SetListener(this);
}

void CategoryTest::UnRegister(string categoryName, AbstractTestInterface* testClass) {

    auto result = std::find(std::begin(_categoryMap[categoryName]), std::end(_categoryMap[categoryName]), testClass);
    if (result != std::end(_categoryMap[categoryName]))
    {
        _categoryMap[categoryName].erase(result);
    }
}
void CategoryTest::HandleComplete() {
    _cs.Unlock();
}

void CategoryTest::WaitForCompletion() {

    while(_executionCount > 0 ) { //TODO: Add condition for cancel test.
        if (_cs.Lock(wakeInterval) == Core::ERROR_NONE){
            _executionCount--;
        }
    }
}

CategoryTest::~CategoryTest() {
  TestManager::Instance().UnRegisterTest(this);
  for(auto category = _categoryMap.begin(); category != _categoryMap.end(); category++ ){
    if((*category).second.size() > 0) {
        (*category).second.clear();
    }
  }
  _categoryMap.clear();
}

void CategoryTest::ExecuteTest() {
  SetExecutionState(ExecutionState::RUNNING);
  SetExecutionStatus(ExecutionStatus::EXECUTING);
  for(auto category = _categoryMap.cbegin(); category != _categoryMap.cend(); category++ ){
    for(auto test = category->second.begin(); test != category->second.end() && IsTestCancelled() != true; test++) {
      _executionCount++;
      std::cerr<<"Starting Test: "<<(*test)->GetName()<<'\n';
      (*test)->ExecuteTest();
    }   
    WaitForCompletion();
  }
  std::cerr<<"All Tests in Category Completed\n";
  SetExecutionState(ExecutionState::STOPPED);
  SetExecutionStatus(ExecutionStatus::SUCCESS);
  _listener->HandleComplete();
}

void CategoryTest::CancelTest() {
  SetCancelTest(true);
  for(auto category = _categoryMap.cbegin(); category != _categoryMap.cend(); category++ ){
    for(auto test = category->second.begin(); test != category->second.end() && (*test)->GetExecutionState() == ExecutionState::RUNNING; test++) {
        std::cerr<<"Sending cancel request to test :"<<(*test)->GetName()<<'\n';
        (*test)->CancelTest();
    }
  }
}

void CategoryTest::SetCancelTest(bool value) {
    std::lock_guard<std::mutex> lk(_lock);
    _cancelTest = value;
}

bool CategoryTest::IsTestCancelled() const {
    std::lock_guard<std::mutex> lk(_lock);
    return _cancelTest;
}

void CategoryTest::SetListener(HandleNotification* listener) {
    _listener = listener;
}

string CategoryTest::GetName() const {
  string testNames;
  for(auto category: _categoryMap){
    testNames += category.first + "\n===============\n" ;
    for(auto test: category.second){
      testNames += test->GetName() + "\n";
    }
  }
  return testNames;
}


} // namespace StressTest
} // namespace WPEFramework

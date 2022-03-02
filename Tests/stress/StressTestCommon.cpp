#include "StressTestCommon.h"
#include "StressTestAdapters.h"

namespace WPEFramework{
namespace StressTest{

uint32_t MonitorCancelRequest::Worker() {
  TestManager::Instance().StopExecution();
  Stop();
  return Core::infinite;
}


void CategoryTest::SetExecutionState(ExecutionState state) {
    std::lock_guard<std::mutex> lk(_lock);
    _executionState = state;
}

void CategoryTest::Register(string categoryName, TestAdapterBaseClass* testClass) {
    _categoryMap[categoryName].push_back(testClass);
    testClass->SetListener(this);
}

void CategoryTest::UnRegister(string categoryName, TestAdapterBaseClass* testClass) {

    auto result = std::find(std::begin(_categoryMap[categoryName]), std::end(_categoryMap[categoryName]), testClass);
    if (result != std::end(_categoryMap[categoryName]))
    {
        _categoryMap[categoryName].erase(result);
    }
    std::cout<<"Unregister in Categorytest complete\n";
}

ExecutionState CategoryTest::GetExecutionState() const{
    std::lock_guard<std::mutex> lk(_lock);
    return _executionState;
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
  std::cout<<"Category Destructor Begin\n";
  TestManager::Instance().UnRegisterTest(this);
  std::cout<<"Category Unregister complete\n";
  for(auto category = _categoryMap.begin(); category != _categoryMap.end(); category++ ){
    if((*category).second.size() > 0) {
        (*category).second.clear();
    }
  }
  _categoryMap.clear();
  std::cout<<"Category Destructor complete\n";
}

void CategoryTest::ExecuteTest() {
  SetExecutionState(ExecutionState::RUNNING);
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
  _listener->HandleComplete();
}

void CategoryTest::CancelTest() {
  std::cerr<<"Received CancelTest Request\n";
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
    return "CategoryTest";
}


} // namespace StressTest
} // namespace WPEFramework

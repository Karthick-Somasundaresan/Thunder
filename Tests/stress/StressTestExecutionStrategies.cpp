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
#include "StressTestExecutionStrategies.h"
#include "StressTestManager.h"


namespace WPEFramework {
namespace StressTest {



//Start of Strategies

void LinearTimedTaskExecutor::ExecuteTest() {
  Core::Time nextTick = Core::Time::Now();
  //TODO: start the timer over here
  _timer.Schedule(nextTick.Ticks(), TimerHandler(this));
  return;
}

uint64_t LinearTimedTaskExecutor::Timed(const uint64_t scheduledTime) {
  Core::Time nextTick = Core::Time::Now();
  uint32_t time = _trafficGenerator->getNextTimerValue() * 1000;
  uint32_t objCount = _trafficGenerator->getObjectCount();
  int32_t delta = _lastObjectReqCount - objCount;
  uint64_t retVal = 0;
  _report.insert(std::pair<uint64_t, int32_t>(nextTick.Ticks(), delta * -1));
  if (_lastObjectReqCount > objCount) {
    //Remove the delta from the list
    _loadTestInterface->DecreaseStress(abs(delta));
  }
  else {
    //Request the delta from the Pool and add it to the pool
    _loadTestInterface->IncreaseStress(abs(delta));
  }
  _lastObjectReqCount = objCount;
  if(time !=0 && _cancelTest != true){
    nextTick.Add(time);
    retVal = nextTick.Ticks();
  }
  else {
    if(_parent != nullptr) {
      _parent->NotifyTestComplete(_report);
    }
  }
  return retVal;
}
void LinearTimedTaskExecutor::Cleanup() {
  _loadTestInterface->Cleanup();
  return;
}
void LinearTimedTaskExecutor::CancelTest(){
  _cancelTest = true;
  return;
}

void OnlyLoadThreadTaskExecutor::ExecuteTest() {
  
  Core::Time nextTick = Core::Time::Now();
  //TODO: start the timer over here
  _timer.Schedule(nextTick.Ticks(), TimerHandler(this));
  while(_cancelTest != true){
    _loadTestInterface->IncreaseStress();
  }
  _parent->NotifyTestComplete();
}
void OnlyLoadThreadTaskExecutor::CancelTest() {
  _cancelTest = true;
}
uint64_t OnlyLoadThreadTaskExecutor::Timed(uint64_t scheduledTime) {
  _cancelTest = true;
  return 0;
}
void OnlyLoadThreadTaskExecutor::Cleanup() {
  _loadTestInterface->Cleanup();
  return;
}


void OnlyUnLoadThreadTaskExecutor::ExecuteTest() {
  while(_cancelTest != true){
    _loadTestInterface->DecreaseStress();
  }
  _parent->NotifyTestComplete();
}

void OnlyUnLoadThreadTaskExecutor::CancelTest() {
  _cancelTest = true;
}
void OnlyUnLoadThreadTaskExecutor::Cleanup() {
  _loadTestInterface->Cleanup();
  return;
}
uint64_t OnlyUnLoadThreadTaskExecutor::Timed(uint64_t scheduledTime) {
  _cancelTest = true;
  return 0;
}

//End of Strategies
}
}
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

#ifndef __STRESS_TEST_EXECUTION_STRATEGIES__
#define __STRESS_TEST_EXECUTION_STRATEGIES__
#include<iostream>
#include <core/core.h>
#include <vector>
#include <TypeTraits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <algorithm>
#include "StressTestUtil.h"
#include "StressTestTrafficGenerator.h"


namespace WPEFramework {
namespace StressTest {



//Start of Strategies
struct TaskExecutionInterface {
    virtual void ExecuteTest() = 0;
    virtual void CancelTest() = 0;
    virtual void Cleanup() = 0;
    virtual ~TaskExecutionInterface() = default;
};

struct TimedTaskExecutionInterface: public TaskExecutionInterface {
    virtual uint64_t Timed(const uint64_t scheduledTime) = 0;
    virtual ~TimedTaskExecutionInterface() = default;
};
class TimerHandler {
  public:
    TimerHandler() = delete;
    TimerHandler(const TimerHandler&) = default;
    TimerHandler& operator=(const TimerHandler&) = delete;
    ~TimerHandler() = default;
    TimerHandler(TimedTaskExecutionInterface* parent): _parent(parent){

    }
    uint64_t Timed(const uint64_t scheduledTime) {
      return _parent->Timed(scheduledTime);
    }
  private:
    TimedTaskExecutionInterface* _parent;
};
class LinearTimedTaskExecutor: public TimedTaskExecutionInterface {
  public: 
    LinearTimedTaskExecutor() = delete;
    LinearTimedTaskExecutor(LinearTimedTaskExecutor& ) = delete;
    LinearTimedTaskExecutor& operator= (const LinearTimedTaskExecutor&) = delete;
    ~LinearTimedTaskExecutor() = default;
    LinearTimedTaskExecutor(TestManagerInterface* parent, LoadTestInterface* loadTestObj, TrafficGeneratorInterface* trafficGen): _parent(parent)
                                                                                                                                , _loadTestInterface(loadTestObj)
                                                                                                                                , _trafficGenerator(trafficGen)
                                                                                                                                , _lastObjectReqCount(0)
                                                                                                                                , _report()
                                                                                                                                , _timer(Core::Thread::DefaultStackSize(), _T(trafficGen->getName().c_str()))
                                                                                                                                , _cancelTest(false)
    {

    }
    void ExecuteTest() override;
    uint64_t Timed(const uint64_t scheduledTime);
    void Cleanup() override;
    void CancelTest() override;
  private:
    TestManagerInterface* _parent;
    LoadTestInterface* _loadTestInterface;
    TrafficGeneratorInterface* _trafficGenerator;
    uint32_t _lastObjectReqCount;
    std::map<uint64_t, int32_t> _report;
    Core::TimerType<TimerHandler> _timer;
    bool _cancelTest;
};


class OnlyLoadThreadTaskExecutor: public TimedTaskExecutionInterface {
  public:
    OnlyLoadThreadTaskExecutor() = delete;
    OnlyLoadThreadTaskExecutor(OnlyLoadThreadTaskExecutor& ) = delete;
    OnlyLoadThreadTaskExecutor& operator= (const OnlyLoadThreadTaskExecutor&) = delete;
    OnlyLoadThreadTaskExecutor(TestManagerInterface* parent, LoadTestInterface* loadTestObj, uint32_t duration): _parent(parent)
                                                                                            , _loadTestInterface(loadTestObj)
                                                                                            , _timer(Core::Thread::DefaultStackSize(), _T("LoadStopTimer"))
                                                                                            , _cancelTest(false)
                                                                                            , _duration(duration)
    {

    }
    ~OnlyLoadThreadTaskExecutor() = default;
    void ExecuteTest() override;
    void CancelTest() override;
    uint64_t Timed(uint64_t scheduledTime);
    void Cleanup() override;
  private:
    TestManagerInterface* _parent;
    LoadTestInterface* _loadTestInterface;
    Core::TimerType<TimerHandler> _timer;
    bool _cancelTest;
    uint32_t _duration;
};

class OnlyUnLoadThreadTaskExecutor: public TimedTaskExecutionInterface {
  public:
    OnlyUnLoadThreadTaskExecutor() = delete;
    OnlyUnLoadThreadTaskExecutor(OnlyUnLoadThreadTaskExecutor& ) = delete;
    OnlyUnLoadThreadTaskExecutor& operator= (const OnlyUnLoadThreadTaskExecutor&) = delete;
    OnlyUnLoadThreadTaskExecutor(TestManagerInterface* parent, LoadTestInterface* loadTestObj, uint32_t duration): _parent(parent)
                                                                                            , _loadTestInterface(loadTestObj)
                                                                                            , _timer(Core::Thread::DefaultStackSize(), _T("UnLoadStopTimer"))
                                                                                            , _cancelTest(false)
                                                                                            , _duration(duration)
    {

    }
    ~OnlyUnLoadThreadTaskExecutor() = default;

    void ExecuteTest() override;
    void CancelTest() override;
    void Cleanup() override;
    uint64_t Timed(uint64_t scheduledTime);
  private:
    TestManagerInterface* _parent;
    LoadTestInterface* _loadTestInterface;
    Core::TimerType<TimerHandler> _timer;
    bool _cancelTest;
    uint32_t _duration;
};

//End of Strategies
}
}
#endif
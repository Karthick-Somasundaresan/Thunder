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

#pragma once
#include "StressTestCommon.h"
namespace WPEFramework{
namespace StressTest
{


template<typename TESTCLASS, typename EXECUTOR>
class AdaptorWithExecutor: public TestAdapterBaseClass, public HandleNotification {
  public:
    template<typename... ARGS>
      AdaptorWithExecutor(TESTCLASS& testClass, ARGS&&... args) : _testClass(testClass)
                                                                , _executor(this, std::forward<ARGS>(args)...)
                                                                , _listener(nullptr)
                                                                , _executionState(ExecutionState::STOPPED)
                                                                , _executionStatus(ExecutionStatus::NOTEXECUTED)
                                                                , _lock(){

      }
      void ExecuteTest() override {
        SetExecutionState(ExecutionState::RUNNING);
        SetExecutionStatus(ExecutionStatus::EXECUTING);
        _executor.StartExecution();
      }
      void CancelTest() override {
        SetExecutionState(ExecutionState::STOPPED);
        SetExecutionStatus(ExecutionStatus::CANCELLED);
        _executor.CancelExecution();
        _testClass.Cleanup();
      }
      void HandleChange(Direction direction, uint32_t value) override{
        if(direction == Direction::INCREASE) {
          _testClass.IncreaseLoad(value);
        } else {
          _testClass.DecreaseLoad(value);
        }
      }
      void HandleComplete() override{
        _listener->HandleComplete();
        _testClass.Cleanup();
        SetExecutionStatus(ExecutionStatus::SUCCESS);
      }
      string GetName() const override {
        return  _testClass.GetClassName() + " executing with: " + _executor.GetName();;
      }
      void SetListener(HandleNotification* listener) {
        _listener = listener;
      }

      ExecutionState GetExecutionState() const override {
          std::lock_guard<std::mutex> lk(_lock);
          return _executionState;
      }

      void SetExecutionState(ExecutionState state) {
          std::lock_guard<std::mutex> lk(_lock);
          _executionState = state;
      }

      ExecutionStatus GetExecutionStatus() const{
          return _executionStatus;
      }

    private:
      TESTCLASS& _testClass;
      EXECUTOR _executor;
      HandleNotification* _listener;
      ExecutionState _executionState;
      ExecutionStatus _executionStatus;
      mutable std::mutex _lock;
};

//This class will be inherited by Test cases which will directly register with TestManager.
template <typename TESTCLASS>
class SimpleAdapter: public TestAdapterBaseClass, public HandleNotification {
  public:
    SimpleAdapter(TESTCLASS& testClassRef): _testClassRef(testClassRef), _listener(nullptr) {

    }
    virtual void ExecuteTest() = 0;
    virtual void CancelTest() = 0 ;
    virtual void HandleChange(Direction direction, uint32_t value ) = 0;
    virtual void HandleComplete() = 0;
    virtual void SetNotification(HandleNotification* listner) = 0;
    virtual ExecutionState GetExecutionState() = 0;
    virtual string GetName() const = 0;
  private:
    TESTCLASS& _testClassRef;
    HandleNotification* _listener;
};


template<typename SHAPER>
class NonIntervalBasedExecutor {
  private:
    class Action{
      private:

      public:
        Action() = delete;
        Action(const Action& other): _listener(other._listener), _shaper(other._shaper){

        }
        Action& operator=(const Action&) = delete;
        Action(HandleNotification* listener, SHAPER* shaper): _listener(listener)
                                                            , _shaper(shaper) {

        }
        bool operator==(const Action& RHS) const {
            return (_listener == RHS._listener && _shaper == RHS._shaper);
        }
        uint64_t Timed(uint64_t scheduledTime){
          Direction direction = _shaper->GetValue() == 1?Direction::INCREASE: Direction::DECREASE;

          Core::Time expiryTime = Core::Time::Now().Add(_shaper->GetNextTimerValue());

          while(Core::Time::Now() < expiryTime){
            _listener->HandleChange(direction, 1);
          }
          std::cerr<<"Test Completed\n";
          _listener->HandleComplete();
          return 0;

        }
        
      private:
        HandleNotification* _listener;
        SHAPER* _shaper;
    };
  public:
    template<typename... Args>
    NonIntervalBasedExecutor(HandleNotification* listener, Args&&... args): _listener(listener)
                                                                  , _shaper(std::forward<Args>(args)...)
                                                                //   , _action(_listener, _shaper)
                                                                  , _actionTimer(Core::Thread::DefaultStackSize(), _T(_shaper.GetName().c_str())){

    }
    void StartExecution() {
    //   Core::Time expiryTime = Core::Time::Now().Add(_shaper.GetNextTimerValue());
    //   _actionTimer.Schedule(Core::Time::Now().Ticks(), _action);
      _actionTimer.Schedule(Core::Time::Now().Ticks(), Action(_listener, &_shaper));

    }

    void CancelExecution () {
        // _actionTimer.Revoke(_action);
        std::cerr<<"Cancelling Execution on NonIntervalBasedExecutor\n";
        _actionTimer.Revoke(Action(_listener, &_shaper));
        std::cerr<<"After Cancelling Execution on NonIntervalBasedExecutor\n";
        _listener->HandleComplete();
    }

    string GetName() const {
        return "NonIntervalBasedExecutor with " + _shaper.GetName();
    }

  private:
    HandleNotification* _listener;
    SHAPER _shaper;
    // Action _action;
    Core::TimerType<Action> _actionTimer;

};

template<typename SHAPER>
class IntervalBasedExecutor {
  private:
    class TimerHandler {
      public:
        TimerHandler() = delete;
        TimerHandler(HandleNotification* listener, SHAPER* trafficShaper): _listener(listener), _trafficShaper(trafficShaper), _lastValue(0) {
        }
        bool operator==(const TimerHandler& RHS) const {
            return (_listener == RHS._listener && _trafficShaper == RHS._trafficShaper);
        }
        uint64_t Timed(uint64_t scheduledTime) {
          Core::Time nextTick = Core::Time::Now();
          uint32_t nextSchedule = _trafficShaper->GetNextTimerValue();
          uint32_t currentValue = _trafficShaper->GetValue();
          int32_t delta = currentValue - _lastValue;
          uint64_t retVal = 0;

          if (delta > 0) {
            _listener->HandleChange(Direction::INCREASE, abs(delta));
          } else {
            _listener->HandleChange(Direction::DECREASE, abs(delta));
          }
          if(nextSchedule) {
            nextTick.Add(nextSchedule);
            retVal = nextTick.Ticks();
            _lastValue = currentValue;
          } else {
            _trafficShaper->Reset();
            _lastValue = 0;
            _listener->HandleComplete();
          }
          return retVal;
        }
      private:
        HandleNotification* _listener;
        SHAPER* _trafficShaper;
        uint32_t _lastValue;
    };
  public:
    IntervalBasedExecutor() = delete;
    template <typename... Args>
    IntervalBasedExecutor(HandleNotification* listener, Args&&... args ): _listener(listener)
                                                                , _trafficShaper(std::forward<Args>(args)...)
                                                                // , _timerHandler(_listener, _trafficShaper)
                                                                , _timer(Core::Thread::DefaultStackSize(), _T(_trafficShaper.GetName().c_str())){

    }
    IntervalBasedExecutor(const IntervalBasedExecutor&) = delete;
    IntervalBasedExecutor& operator=(const IntervalBasedExecutor&) = delete;
    void StartExecution() {
      _timer.Schedule(Core::Time::Now().Ticks(), TimerHandler(_listener, &_trafficShaper));
    }

    void CancelExecution () {
        // _timer.Revoke(_timerHandler);
        std::cerr<<"Cancelling Execution on IntervalBasedExecutor\n";
        _timer.Revoke(TimerHandler(_listener, &_trafficShaper));
        std::cerr<<"After Cancelling Execution on IntervalBasedExecutor\n";
        _listener->HandleComplete();
    }

    string GetName() const {
        return "IntervalBasedExecutor and " + _trafficShaper.GetName();
    }
  private:
    HandleNotification* _listener;
    SHAPER _trafficShaper;
    // TimerHandler _timerHandler;
    Core::TimerType<TimerHandler> _timer;
};

// template<typename EXECUTOR>
// class TestBaseWithExecutor: public AdaptorWithExecutor<TestBaseWithExecutor<EXECUTOR>, EXECUTOR>, public LoadTestInterface {
//   public:
//     virtual void ExecuteTest() = 0;
//     virtual void CancelTest() = 0;
//     virtual void HandleChange(Direction, uint32_t) = 0;
//     virtual void HandleComplete() = 0;
//     virtual string GetName() = 0;
//     virtual ExecutionState GetState() = 0;
//     virtual 

// };

} // namespace StressTest
} // namespace WPEFramework

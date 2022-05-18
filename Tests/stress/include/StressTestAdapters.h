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
#include "StressTestMacros.h"

namespace WPEFramework{
namespace StressTest
{

struct ExecutorInterface {
  virtual void StartExecution() = 0 ;
  virtual void CancelExecution() = 0 ;
  virtual void AddListener(HandleNotification*) = 0;
  virtual string GetName() const = 0;
  virtual ~ExecutorInterface() = default;
};


struct LoadGeneratorInterface {
  virtual uint64_t GetNextTimerValue(uint64_t elapsedTime) = 0;
  virtual uint32_t GetLoadValue(uint64_t elapsedTime) = 0;
  virtual string GetName() const = 0 ;
  virtual ~LoadGeneratorInterface() = default;
};

class LoadGeneratorRegistry{
  private:
    class AbstractLoadGenerator : public LoadGeneratorInterface {
      public:
        DELETE_DEFAULT_AND_COPIES(AbstractLoadGenerator);
        AbstractLoadGenerator(uint32_t duration, uint32_t freq, uint32_t threshold, uint32_t stepValue, string waveName): _duration(duration)
                                                                                                                        , _freq(freq)
                                                                                                                        , _maxThreshold(threshold)
                                                                                                                        , _stepValue(stepValue)
                                                                                                                        , _waveName(waveName) {

        }
        string GetName() const override{
          return _waveName;
        }
        uint64_t GetNextTimerValue(uint64_t elapsedDuration ) override {
          uint64_t val = ((elapsedDuration > (_duration * 1000))? 0 : _stepValue) * 1000;
          return val;
        }
      protected:
        uint32_t _duration;
        uint32_t _freq;
        uint32_t _maxThreshold;
        uint32_t _stepValue;
        string _waveName;
    };

    class TriangleLoadGenerator : public AbstractLoadGenerator {
      public:
        TriangleLoadGenerator& operator=(const TriangleLoadGenerator&) = delete;
        TriangleLoadGenerator(const TriangleLoadGenerator&) = delete;
        TriangleLoadGenerator(): AbstractLoadGenerator(ConfigReader::Instance().Duration()
                                                        , ConfigReader::Instance().Freq()
                                                        , ConfigReader::Instance().ProxyCategoryMaxThreshold()
                                                        , 1, "TrianglularWave"){}
        TriangleLoadGenerator(uint32_t stepValue): AbstractLoadGenerator(ConfigReader::Instance().Duration()
                                                        , ConfigReader::Instance().Freq()
                                                        , ConfigReader::Instance().ProxyCategoryMaxThreshold()
                                                        , stepValue, "StepWave"){}
        uint32_t GetLoadValue(uint64_t elapsedDuration) override {
          uint32_t period = _duration / (_freq * 2);
          uint32_t value = (_maxThreshold/period) * (period - abs( (int32_t)((elapsedDuration % (2 * period)) - period)));
          return value;
        }
        ~TriangleLoadGenerator() = default;
    };

    class SineLoadGenerator : public AbstractLoadGenerator{
      public:
        SineLoadGenerator& operator=(const SineLoadGenerator&) = delete;
        SineLoadGenerator(const SineLoadGenerator&) = delete;
        SineLoadGenerator():AbstractLoadGenerator(ConfigReader::Instance().Duration()
                                                        , ConfigReader::Instance().Freq()
                                                        , ConfigReader::Instance().ProxyCategoryMaxThreshold()
                                                        , 1, "SineWave"){}
        ~SineLoadGenerator() = default;

        uint32_t GetLoadValue(uint64_t elapsedDuration) override {
           return ((_maxThreshold/2) * sin(elapsedDuration * _freq)) + (_maxThreshold/2);
        }
    };

  public:
    static std::unique_ptr<LoadGeneratorInterface> GetLoadGenerator(uint32_t index) {
      std::unique_ptr<LoadGeneratorInterface> loadGeneratorInterface = nullptr;
      switch(index % 3) {
        case 0:
          loadGeneratorInterface = std::unique_ptr<TriangleLoadGenerator>(new TriangleLoadGenerator());
          break;
        case 1:
          loadGeneratorInterface = std::unique_ptr<TriangleLoadGenerator>(new TriangleLoadGenerator(5));
          break;
        case 2:
          loadGeneratorInterface = std::unique_ptr<SineLoadGenerator>(new SineLoadGenerator());
          break;
      }
      return loadGeneratorInterface;
    }

};


class TestAdapter: public AbstractTestInterface, public HandleNotification {
  public:
    DELETE_DEFAULT_AND_COPIES(TestAdapter);
    TestAdapter(StressTestInterface& testClass, ExecutorInterface* executor) : _testClass(testClass)
                                                      , _executor(executor)
                                                      , _listener(nullptr){

      _executor->AddListener(this);
    }
    void ExecuteTest() override {
      SetExecutionState(ExecutionState::RUNNING);
      SetExecutionStatus(ExecutionStatus::EXECUTING);
      _executor->StartExecution();
    }


    void CancelTest() override {
      SetExecutionState(ExecutionState::STOPPED);
      SetExecutionStatus(ExecutionStatus::CANCELLED);
      _executor->CancelExecution();
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
      SetExecutionState(ExecutionState::STOPPED);
      SetExecutionStatus(ExecutionStatus::SUCCESS);
    }
    virtual void SetListener(HandleNotification* listner) override {
      _listener = listner;
    }
    

    string GetName() const override {
      return _executor->GetName() + " on " + _testClass.GetClassName();
    }

    
  private:
    StressTestInterface& _testClass;
    ExecutorInterface* _executor;
    HandleNotification* _listener;
};


template<typename TESTCLASS>
class LoadTestExecutor : public ExecutorInterface, public HandleNotification {
  private:
    class Action {
      public:
        DELETE_DEFAULT_AND_COPIES(Action);
        Action(HandleNotification* listener, uint32_t id, uint64_t startTime) : _listener(listener)
                                                          , _id(id)
                                                          , _shaper(LoadGeneratorRegistry::GetLoadGenerator(id))
                                                          , _startTime(startTime)
                                                          , _lastValue(0){
        }
        ~Action() = default;
        Action(Action&& other) {
          _listener = other._listener;
          _id = other._id;
          _shaper = std::move(other._shaper);
          _lastValue = other._lastValue;
          _startTime = other._startTime;
        }
        
        uint64_t Timed(uint64_t scheduledTime) {
          uint64_t elapsedDuration = 0;
          if (scheduledTime < _startTime) {
            elapsedDuration = _startTime - scheduledTime + 1;
          } else{
            elapsedDuration = scheduledTime - _startTime;
          }
          Core::Time nextTick = Core::Time::Now();
          uint32_t nextSchedule = _shaper->GetNextTimerValue(elapsedDuration/1000);
          uint32_t currentValue = _shaper->GetLoadValue(elapsedDuration/1000);
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
            _lastValue = 0;
            std::cout<<"Load Test Completed\n";
            _listener->HandleComplete();
          }
          return retVal;
        }
        bool operator==(const Action& other) {
          return (_listener == other._listener && _shaper == other._shaper && _id == other._id);
        }
        private:
          HandleNotification* _listener;
          uint32_t _id;
          std::unique_ptr<LoadGeneratorInterface> _shaper;
          uint64_t _startTime;
          uint32_t _lastValue;
    };
  public:
    DELETE_DEFAULT_AND_COPIES(LoadTestExecutor);
    template<typename... Args>
    LoadTestExecutor(uint32_t noOfThreads, bool customThreads, string category, Args&&... args): _noOfThreads(noOfThreads)
                                                          , _customThreads(customThreads)
                                                          , _executionCount(0)
                                                          , _cs()
                                                          , _listener(nullptr)
                                                          , _testClass(std::forward<Args>(args)...)
                                                          , _adapter(_testClass, this)
                                                          , _category(category)
                                                          // , _listOfShapers{new TriangleTrafficGenerator(), new TriangleTrafficGenerator(5), new SineTrafficGenerator()}
                                                          , _listOfTimers(){
      ASSERT(_noOfThreads > 0);
      if(category == "Individual") {
        TestManager::Instance().RegisterTest(&_adapter);
      } else {
        CategoryTest::Instance().Register(category, &_adapter);
      }
      for(uint32_t index = 0; index < _noOfThreads; index++) {
            _listOfTimers.emplace_back(new Core::TimerType<Action>(Core::Thread::DefaultStackSize(), _T(_testClass.GetClassName().c_str())));
        }
    }
    void StartExecution() override {
      for(uint32_t index = 0; index < _listOfTimers.size(); index++){
        uint64_t startTime = Core::Time::Now().Ticks();
        _executionCount++;
        _listOfTimers[index]->Schedule(startTime, Action( this, index, startTime));
      }
    }
    void CancelExecution() override {
      for(unsigned int index = 0; index < _listOfTimers.size(); index++){
        _listOfTimers[index]->Flush();
      }
      _listener->HandleComplete();
    }
    void AddListener(HandleNotification* listener) override{
      _listener = listener;
    }
    string GetName() const override{
      if(_customThreads){
        return "Load Test With Custom threads ";
      } else {
        return "Load Test ";
      }
    }
    void HandleChange(Direction direction, uint32_t value) override {
      _listener->HandleChange(direction, value);
    }
    void HandleComplete() override {
      _cs.Lock();
      _executionCount--;
      _cs.Unlock();
      if(_executionCount == 0) {
        _listener->HandleComplete();
      }
    }
    ~LoadTestExecutor() {
      _listOfTimers.clear();
      if(_category == "Individual") {
        TestManager::Instance().UnRegisterTest(&_adapter);
      } else {
        CategoryTest::Instance().UnRegister(_category, &_adapter);
      }
    }
  private:
    uint32_t _noOfThreads;
    bool _customThreads;
    uint32_t _executionCount;
    Core::CriticalSection _cs;
    HandleNotification* _listener;
    TESTCLASS _testClass;
    TestAdapter _adapter;
    string _category;
    std::vector<Core::TimerType<Action>*> _listOfTimers;

};

template <typename TESTCLASS>
class StressTestExecutor : public ExecutorInterface , public HandleNotification{
  private:
    class ActionThread : public Core::Thread {
      public:
        DELETE_DEFAULT_AND_COPIES(ActionThread);
        ActionThread(HandleNotification* listener, Direction direction, uint32_t duration): Core::Thread(Core::Thread::DefaultStackSize(), nullptr)
                                                                                          , _listener(listener)
                                                                                          , _direction(direction)
                                                                                          , _duration(duration){
        }
        ~ActionThread() {
          Stop();
          Wait(Thread::BLOCKED | Thread::STOPPED | Thread::INITIALIZED, Core::infinite);
        }
        uint32_t Worker() override {
          const Core::Time expiryTime = Core::Time::Now().Add(_duration* 1000);

          while(IsRunning() && Core::Time::Now() < expiryTime){
            _listener->HandleChange(_direction, 1);
            std::this_thread::yield();
          }
          std::cout<<"Stress Test Complete\n";
          _listener->HandleComplete();
          Block();
          return Core::infinite;
        }
      private:
        HandleNotification* _listener;
        Direction _direction;
        uint32_t _duration;
    };
  public:
    DELETE_DEFAULT_AND_COPIES(StressTestExecutor);
    template<typename... Args>
    StressTestExecutor(uint32_t noOfThreads, bool customThreads, string category, Args&&... args) : _noOfThreads((noOfThreads % 2) ? noOfThreads + 1 : noOfThreads)
                                                                              , _customThreads(0)
                                                                              , _executionCount(0)
                                                                              , _cs()
                                                                              , _category(category)
                                                                              , _listener(nullptr)
                                                                              , _testClass(std::forward<Args>(args)...)
                                                                              , _adapter(_testClass, this)
                                                                              , _listOfThreads() {

      ASSERT(_noOfThreads > 0);
      if(category == "Individual") {
        TestManager::Instance().RegisterTest(&_adapter);
      } else {
        CategoryTest::Instance().Register(category, &_adapter);
      }
      for(uint32_t index = 0; index < _noOfThreads; index++) {
        _listOfThreads.emplace_back(new ActionThread(this, (index % 2 ? Direction::INCREASE : Direction::DECREASE), ConfigReader::Instance().Duration()));
      }
    }
    void HandleChange(Direction direction, uint32_t value) {
      _listener->HandleChange(direction, value);
    }
    void HandleComplete() {
      _cs.Lock();
      _executionCount--;
      _cs.Unlock();
      if (_executionCount == 0) {
        _listener->HandleComplete();
      }
    }

    void StartExecution() override {
      for(uint32_t index = 0; index < _listOfThreads.size(); index++){
        _cs.Lock();
        _executionCount++;
        _cs.Unlock();
        _listOfThreads[index]->Run();
      }
    }
    void CancelExecution() override {
      for(unsigned int index = 0; index < _listOfThreads.size(); index++){
        _listOfThreads[index]->Stop();
      }
      _listener->HandleComplete();
    }
    void AddListener(HandleNotification* listener) override{
      _listener = listener;
    }

    string GetName() const override{
      if(_customThreads) {
        return "Stress Test With Custom Threads ";
      } else {
        return "Stress Test ";
      }
    }
    ~StressTestExecutor() {
      // _listOfTimers.clear();
      for(auto& thread : _listOfThreads) {
        delete thread;
      }
      _listOfThreads.clear();
      if(_category == "Individual") {
        TestManager::Instance().UnRegisterTest(&_adapter);
      } else {
        CategoryTest::Instance().UnRegister(_category, &_adapter);
      }
    }
  private:
    uint32_t _noOfThreads;
    bool _customThreads;
    uint32_t _executionCount;
    Core::CriticalSection _cs;
    string _category;
    HandleNotification* _listener;
    TESTCLASS _testClass;
    TestAdapter _adapter;
    std::vector<ActionThread*> _listOfThreads;
};

} // namespace StressTest
} // namespace WPEFramework

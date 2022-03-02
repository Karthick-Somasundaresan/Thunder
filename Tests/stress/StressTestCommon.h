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
#include "Module.h"
#include <mutex>

namespace WPEFramework {
namespace StressTest {

constexpr unsigned int wakeInterval = 500;

class TestManager;

enum class Direction{
  INCREASE,
  DECREASE
};

enum class ExecutionState {
  RUNNING,
  STOPPED
};

enum class ExecutionStatus {
  NOTEXECUTED,
  EXECUTING,
  SUCCESS,
  FAILURE,
  SKIPPED,
  CANCELLED
};

struct HandleNotification {
  virtual void HandleChange(Direction direction, uint32_t value) {}
  virtual void HandleComplete() {}
  virtual ~HandleNotification() = default;
};


class TestInterface {
  public:
    TestInterface(): _executionStatus(ExecutionStatus::NOTEXECUTED){}
    virtual void ExecuteTest() = 0;
    virtual void CancelTest() = 0;
    virtual void SetListener(HandleNotification*) = 0;
    virtual string GetName() const = 0;
    ExecutionStatus GetExecutionStatus() const {
        return _executionStatus;
    }
    virtual ExecutionState GetExecutionState() const = 0;
    virtual ~TestInterface () = default;
  protected:
    void SetExecutionStatus(ExecutionStatus executionStatus) {
      _executionStatus = executionStatus;
    }
  private:
    ExecutionStatus _executionStatus;

};

struct LoadTestInterface {
  virtual void IncreaseLoad(uint32_t loadFactor) = 0;
  virtual void DecreaseLoad(uint32_t loadFactor) = 0;
  virtual void MaxLoad() = 0;
  virtual void NoLoad() = 0;
  virtual string GetClassName() const = 0;
  virtual void Cleanup() = 0;
  virtual ~LoadTestInterface() = default;
};



class MonitorCancelRequest: public WPEFramework::Core::Thread {
    public:
    MonitorCancelRequest() = default;
    ~MonitorCancelRequest() = default;
    uint32_t Worker();
};
class TestManager: public HandleNotification{
  private:
    TestManager():_listOfTests(), _executionCount(-1), _cs(0,1),_monitorCancelRequestThread(), _cancelTest(false), _lock() {
        _monitorCancelRequestThread.Suspend();
    }
    void WaitForCompletion();
  public:
    TestManager(const TestManager&) = delete;
    ~TestManager();
    TestManager& operator=(const TestManager&) = delete;
    static TestManager& Instance();
    void RegisterTest(TestInterface*);
    void UnRegisterTest(TestInterface*);
    void PerformTest();
    void HandleComplete() override;
    void StopExecution();
    void HandleCancelRequest();
  private:
    bool IsTestCancelled() const;
    void CancelTest();
  private:
    std::vector<TestInterface*> _listOfTests;
    unsigned int _executionCount;
    Core::CountingSemaphore _cs;
    MonitorCancelRequest _monitorCancelRequestThread;
    bool _cancelTest;
    mutable std::mutex _lock;

};

struct TestAdapterBaseClass : public TestInterface {
  TestAdapterBaseClass() = default;
  TestAdapterBaseClass& operator=(const TestAdapterBaseClass&) = delete;
  TestAdapterBaseClass(const TestAdapterBaseClass&) = delete;
  virtual ~TestAdapterBaseClass() {}
  virtual void ExecuteTest() = 0;
  virtual void CancelTest() = 0;
  virtual ExecutionState GetExecutionState() const = 0;
  virtual string GetName() const = 0;
  virtual bool Validate() = 0;
};

class CategoryTest : public TestInterface, public HandleNotification {

    protected:
        CategoryTest():_categoryMap(), _listener(nullptr), _cs(0, UINT32_MAX), _cancelTest(false), _executionCount(0), _executionState(ExecutionState::STOPPED), _lock() {
            TestManager::Instance().RegisterTest(this);
        }
    public:
        static CategoryTest& Instance() {
            static CategoryTest pct;
            return pct;
        }
        void Register(string, TestAdapterBaseClass*);
        void UnRegister(string, TestAdapterBaseClass*);
        void ExecuteTest() override;
        void CancelTest() override;
        string GetName() const override;
        void SetListener(HandleNotification* listner) override;
        void HandleComplete() override;
        ExecutionState GetExecutionState() const override;
        ~CategoryTest();
    private:
        void SetCancelTest(bool);
        bool IsTestCancelled() const;
        void WaitForCompletion();
        void SetExecutionState(ExecutionState);

    private:
        std::map<string, std::vector<TestAdapterBaseClass* >> _categoryMap;
        HandleNotification* _listener;
        Core::CountingSemaphore _cs;
        bool _cancelTest;
        uint32_t _executionCount;
        ExecutionState _executionState;
        mutable std::mutex _lock;
};


class ConfigReader {
  private:
    class JSONConfig: public Core::JSON::Container {
      public:
        JSONConfig(const JSONConfig& rhs):Core::JSON::Container(), Duration(rhs.Duration), Freq(rhs.Freq), ProxyCategoryMaxThreshold(rhs.ProxyCategoryMaxThreshold){}
        JSONConfig(): Core::JSON::Container(),Duration(), Freq(), ProxyCategoryMaxThreshold(){

          Add(_T("duration"), &Duration);
          Add(_T("freq"), &Freq);
          Add(_T("ProxyCategory_MaxThreshold"), &ProxyCategoryMaxThreshold);
        }
        ~JSONConfig() override = default;

      public:
        Core::JSON::DecUInt32 Duration;
        Core::JSON::DecUInt32 Freq;
        Core::JSON::DecUInt32 ProxyCategoryMaxThreshold;
    };

  private:
    ConfigReader():_file(), _duration(60), _freq(8), _proxyCategoryMaxThreshold(1024) {
      char* path = getenv("STRESS_TEST_CONFIG_FILE");
      if (path != nullptr) {
        _file = path;
        ParseConfig();
      } else {
        std::cerr<<"STRESS_TEST_CONFIG_FILE not set. Setting all default values. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold<< '\n';
      }
    }
  public:
    static ConfigReader& Instance() {
      static ConfigReader cr;
      return cr;
    }
    inline uint32_t Duration() const { return _duration;}
    inline uint32_t Freq() const { return _freq;}
    inline uint32_t ProxyCategoryMaxThreshold() const { return _proxyCategoryMaxThreshold;}
  private:
    void ParseConfig() {
      if(_file.Open(true) == true) {
        JSONConfig config;
        Core::OptionalType<Core::JSON::Error> error;
        std::cerr<<"Config readng from file\n";
        config.IElement::FromFile(_file, error);
        if(error.IsSet() == false) {
          if(config.Duration.IsSet()) {
            _duration = config.Duration.Value();
          } else {
            std::cerr<<"Duration not set so setting default value to 60 secs\n";
            _duration = 60;
          }
          if(config.Freq.IsSet()) {
            _freq = config.Freq.Value();
          } else {
            std::cerr<<"Freq not set so setting default value to 8\n";
            _freq = 8;
          }
          if(config.ProxyCategoryMaxThreshold.IsSet()){
            _proxyCategoryMaxThreshold = config.ProxyCategoryMaxThreshold.Value();
          } else {
            std::cerr<<"ProxyCategory_MaxThreshold not set so setting default value to 1024\n";
            _proxyCategoryMaxThreshold = 1024;
          }
          std::cerr<<"File Parsed. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold<< '\n';
        } else {
          std::cerr<<"File Parse Error. Setting all default values. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold<< '\n';
        }
      } else {
        std::cerr<<"Unable to open file. Setting all default values. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold<< '\n';
      }
    }
  private:
    Core::File _file;
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _proxyCategoryMaxThreshold;
};



class TriangleTrafficGenerator{
  public:
    // TriangleTrafficGenerator() = delete;
    TriangleTrafficGenerator(const TriangleTrafficGenerator&) = default;
    TriangleTrafficGenerator& operator=(const TriangleTrafficGenerator&) = delete;
    TriangleTrafficGenerator():TriangleTrafficGenerator(ConfigReader::Instance().Duration()
                                                        , ConfigReader::Instance().Freq()
                                                        , ConfigReader::Instance().ProxyCategoryMaxThreshold()
                                                        , 1, "TrianglularWave"){}
    TriangleTrafficGenerator(uint32_t stepValue):TriangleTrafficGenerator(ConfigReader::Instance().Duration()
                                                        , ConfigReader::Instance().Freq()
                                                        , ConfigReader::Instance().ProxyCategoryMaxThreshold()
                                                        , stepValue, "StepWave"){}
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount): 
        TriangleTrafficGenerator(duration, freq, maxObjectCount, 1, "TriangularWave") {
    }
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount, uint32_t stepValue): 
        TriangleTrafficGenerator(duration, freq, maxObjectCount, stepValue, "StepWave") {
    }
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount, uint32_t stepValue, string wavename);
    ~TriangleTrafficGenerator() = default;


    uint32_t GetNextTimerValue();
    uint32_t GetValue();
    const string GetName() const ;
    void Reset() { _elapsedDuration = 0;}
  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxThreshold;
    uint32_t _elapsedDuration;
    uint32_t _stepValue;
    string _waveName;
    std::map<uint64_t, int32_t> _report;
};


class SineTrafficGenerator {
  public:
    SineTrafficGenerator(const SineTrafficGenerator&) = default;
    SineTrafficGenerator& operator=(const SineTrafficGenerator&) = delete;
    SineTrafficGenerator(): SineTrafficGenerator(ConfigReader::Instance().Duration()
                                                , ConfigReader::Instance().Freq()
                                                , ConfigReader::Instance().ProxyCategoryMaxThreshold()){}
    SineTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount): _duration(duration)
                                                                     , _freq(freq)
								     , _maxThreshold(maxObjectCount)
								     , _elapsedTime(0)
								     , _stepValue(1)
								     , _waveName("SineWave") {
    }
    
    ~SineTrafficGenerator() = default;
    uint32_t GetNextTimerValue() ;
    uint32_t GetValue() ;
    const string GetName() const ;
    void Reset() { _elapsedTime = 0;}
  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxThreshold;
    uint32_t _elapsedTime;
    uint32_t _stepValue;
    string _waveName;
};

class ConstantTrafficGenerator {
  public:
    ConstantTrafficGenerator() = delete;
    ConstantTrafficGenerator(const ConstantTrafficGenerator&) = delete;
    ConstantTrafficGenerator& operator=(const ConstantTrafficGenerator&) = delete;
    ConstantTrafficGenerator(Direction direction, uint32_t duration):_value((direction==Direction::INCREASE)?1:-1),_duration(duration * 1000) {

      std::cerr<<"Duration: "<< duration<<'\n';
    }
    ~ConstantTrafficGenerator() = default;
    uint32_t GetNextTimerValue() {
      return _duration;
    }
    int32_t GetValue() {
      return _value;
    }
    const string GetName() const { 
      string dir = (_value == 1)? " (Increase)": " (Decrease)";
      return "ConstantTrafficGen" + dir;
    }
    void Reset() {}
  private:
    int32_t _value;
    uint32_t _duration;
};

}
}
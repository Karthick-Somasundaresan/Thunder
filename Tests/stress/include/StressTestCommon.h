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
#include <ostream>
#include <mutex>
#include <time.h>


namespace WPEFramework {
namespace StressTest {

constexpr unsigned int wakeInterval = 500;

class TestManager;

enum class Direction{
  INCREASE,
  DECREASE
};

enum class ExecutionState {
  NOTSTARTED,
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

enum class ReportType {
  TEST_COUNT,
  TEST_NAMES,
  TEST_STATE,
  TEST_STATUS,
  ALL

};
struct HandleNotification {
  virtual void HandleChange(Direction direction, uint32_t value) {}
  virtual void HandleComplete() {}
  virtual ~HandleNotification() = default;
};


class TimeKeeper{
  public:
    TimeKeeper(): _startTime(0), _endTime(0), _completed(false){}

    void Reset() {
      _startTime = _endTime = 0;
      _completed = false;
    }

    void Start(){
      time(&_startTime);
    }

    void Stop(){
      _completed = true;
      time(&_endTime);

    }

    double GetElapsedTime() {
      double elapsedTime = 0.0;
      if(_completed) {
        elapsedTime = difftime(_endTime, _startTime);
      } else if(_startTime > 0) {
        time_t now;
	time(&now);
	elapsedTime = difftime(now, _startTime);
      } else {
        elapsedTime = -1.0;
      }

      return elapsedTime;
    }
    string GetElapsedTimeStr() {
      string elapsedTimeStr;
      int elapsedTime = GetElapsedTime();
      if(elapsedTime != -1) {
        char buffer[24] = {0};
        snprintf(buffer, sizeof(buffer), "%02d: %02d : %02d" , (int)(elapsedTime/3600), (int)((elapsedTime%3600)/60), (int)(((elapsedTime%3600)%60)));
        elapsedTimeStr.assign(buffer);
      } else {
        elapsedTimeStr.assign("00:00:00");
      }
      return elapsedTimeStr;
    }
  private:
    time_t _startTime;
    time_t _endTime;
    bool _completed;

};


inline string ExecutionStateToString(ExecutionState state ){
  string retStr;
  switch(state){
    case ExecutionState::NOTSTARTED:
      retStr = "Not Started";
      break;
    case ExecutionState::RUNNING:
      retStr = "Running";
      break;
    case ExecutionState::STOPPED:
      retStr = "Stopped";
      break;
  }
  return retStr;
} 

inline string ExecutionStatusToString(ExecutionStatus status) {
  string retStr;
  switch(status) {
    case ExecutionStatus::NOTEXECUTED:
      retStr = "Not Executed";
      break;
    case ExecutionStatus::EXECUTING:
      retStr = "Executing";
      break;
    case ExecutionStatus::SUCCESS:
      retStr = "Success";
      break;
    case ExecutionStatus::FAILURE:
      retStr = "Failure";
      break;
    case ExecutionStatus::SKIPPED:
      retStr = "Skipped";
      break;
    case ExecutionStatus::CANCELLED:
      retStr = "Cancelled";
      break;
  }
  return retStr;
}

class AbstractTestInterface {
  public:
    AbstractTestInterface() : _executionStatus(ExecutionStatus::NOTEXECUTED)
                    , _executionState(ExecutionState::NOTSTARTED)
		    , _timeKeeper()
                    , _lock(){

    }
    virtual void ExecuteTest() = 0;
    virtual void CancelTest() = 0;
    virtual void SetListener(HandleNotification*) = 0;
    virtual string GetName() const = 0;
    virtual void PrintReport(ReportType type) {
      switch(type){
        case ReportType::TEST_COUNT:
          std::cout<<"Test count: 1"<<'\n';
          break;
        case ReportType::TEST_NAMES:
          std::cout<<"Test Name: "<<GetName()<<'\n';
          break;
        case ReportType::TEST_STATE:
          std::cout<<"Test Name: "<<GetName()<<"\nState:"<<ExecutionStateToString(GetExecutionState())<<'\n';
          break;
        case ReportType::TEST_STATUS:
          std::cout<<"Test Name: "<<GetName()<<"\nStatus:"<<ExecutionStatusToString(GetExecutionStatus())<<'\n';
          break;
        case ReportType::ALL:
          std::cout<<" -----------------------------------------------------------------------------------\n";
          std::cout<<"\tTest Name:\t"<<GetName()<<"\n\tState:\t\t"<<ExecutionStateToString(GetExecutionState())<<"\n\tStatus:\t\t"<<ExecutionStatusToString(GetExecutionStatus())<<"\n\tElapsedTime:\t"<< _timeKeeper.GetElapsedTimeStr() << "\n";
          std::cout<<" -----------------------------------------------------------------------------------\n";
          break;
      }
    }
    virtual ExecutionStatus GetExecutionStatus() const {
      return _executionStatus;
    }
    virtual ExecutionState GetExecutionState() const {
      return _executionState;
    }
    void Reset() {
      _executionStatus = ExecutionStatus::NOTEXECUTED;
      _executionState = ExecutionState::NOTSTARTED;
      _timeKeeper.Reset();
    }
    virtual ~AbstractTestInterface () = default;
  protected:
    virtual void SetExecutionStatus(ExecutionStatus executionStatus) {
      std::lock_guard<std::mutex> lk(_lock);
      _executionStatus = executionStatus;
    }

    virtual void SetExecutionState(ExecutionState executionState) {
      std::lock_guard<std::mutex> lk(_lock);
      _executionState = executionState;
      if(executionState == ExecutionState::RUNNING) {
        _timeKeeper.Start();
      }
      if (executionState == ExecutionState::STOPPED) {
        _timeKeeper.Stop();
      }
    }
  private:
    ExecutionStatus _executionStatus{ExecutionStatus::NOTEXECUTED};
    ExecutionState _executionState{ExecutionState::STOPPED};
    TimeKeeper _timeKeeper;
    mutable std::mutex _lock;

};

struct StressTestInterface {
  virtual void IncreaseLoad(uint32_t loadFactor) = 0;
  virtual void DecreaseLoad(uint32_t loadFactor) = 0;
  virtual void MaxLoad() = 0;
  virtual void NoLoad() = 0;
  virtual string GetClassName() const = 0;
  virtual void Cleanup() = 0;
  virtual bool Validate() = 0;
  virtual ~StressTestInterface() = default;
};



class TestManager: public HandleNotification{
  private:
    class PerformTestThread: public Core::Thread {
      public:
        PerformTestThread(TestManager* parent): Core::Thread(Core::Thread::DefaultStackSize(), _T("PerformTestThread")), _parent(parent) {
        }
        uint32_t Worker() {
          _parent->PerformTest();
          Block();
          return Core::infinite;
        }
      private:
        TestManager* _parent;
    };
  private:
    TestManager(): _listOfTests()
                 , _executionCount(0)
		 , _cs(0,1)
		 , _cancelWait(true)
		 , _performTestThread(this)
		 , _cancelTest(false)
		 , _timeKeeper()
		 , _lock() {
        _performTestThread.Block();
    }
    void WaitForCompletion();
  public:
    TestManager(const TestManager&) = delete;
    ~TestManager();
    TestManager& operator=(const TestManager&) = delete;
    static TestManager& Instance();
    void RegisterTest(AbstractTestInterface*);
    void UnRegisterTest(AbstractTestInterface*);
    void PerformTest();
    void StartTest();
    void HandleComplete() override;
    void StopExecution();
    void HandleCancelRequest();
    void PrintReport(ReportType)const;
  private:
    bool IsTestCancelled() const;
    void CancelTest();
  private:
    std::vector<AbstractTestInterface*> _listOfTests;
    unsigned int _executionCount;
    Core::CountingSemaphore _cs;
    Core::BinairySemaphore _cancelWait;
    PerformTestThread _performTestThread;
    bool _cancelTest;
    mutable TimeKeeper _timeKeeper;
    mutable std::mutex _lock;

};

class CategoryTest : public AbstractTestInterface, public HandleNotification {

    protected:
        CategoryTest():_categoryMap(), _listener(nullptr), _cs(0, UINT32_MAX), _cancelTest(false), _executionCount(0), _executionState(ExecutionState::STOPPED), _lock() {
            TestManager::Instance().RegisterTest(this);
        }
    public:
        static CategoryTest& Instance() {
            static CategoryTest pct;
            return pct;
        }
        void Register(string, AbstractTestInterface*);
        void UnRegister(string, AbstractTestInterface*);
        void ExecuteTest() override;
        void CancelTest() override;
        string GetName() const override;
        void SetListener(HandleNotification* listner) override;
        void HandleComplete() override;
        ~CategoryTest();
    private:
        void SetCancelTest(bool);
        bool IsTestCancelled() const;
        void WaitForCompletion();

    private:
        std::map<string, std::vector<AbstractTestInterface* >> _categoryMap;
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
          Add(_T("ThreadPool_MaxJobThreshold"), &ThreadPoolMaxJobThreshold);
        }
        ~JSONConfig() override = default;

      public:
        Core::JSON::DecUInt32 Duration;
        Core::JSON::DecUInt32 Freq;
        Core::JSON::DecUInt32 ProxyCategoryMaxThreshold;
        Core::JSON::DecUInt32 ThreadPoolMaxJobThreshold;
    };

  private:
    ConfigReader():_file(), _duration(60), _freq(8), _proxyCategoryMaxThreshold(1024), _threadPoolMaxJobThreshold(1024) {
      char* path = getenv("STRESS_TEST_CONFIG_FILE");
      if (path != nullptr) {
        _file = path;
        ParseConfig();
      } else {
        //std::cerr<<"STRESS_TEST_CONFIG_FILE not set. Setting all default values. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold << " ThreadPool_MaxJobThreshold: "<<_threadPoolMaxJobThreshold<<'\n';
        std::cerr<<"STRESS_TEST_CONFIG_FILE not set. Setting all default values."<<*this;
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
    inline uint32_t ThreadPoolMaxJobThreshold() const { return _threadPoolMaxJobThreshold;}
    friend std::ostream& operator<<(std::ostream& os, const ConfigReader& configReader) {
    	os<<"\n********************************* Config Start ********************************\n"
    	  <<"\t\tDuration: "<<configReader._duration<<"\n"
          <<"\t\tFrequency: "<<configReader._freq<<"\n"
	  <<"\t\tProxy Category Max Threshold: "<<configReader._proxyCategoryMaxThreshold<<"\n"
	  <<"\t\tThreadPool Max Threshold: "<<configReader._threadPoolMaxJobThreshold<<"\n"
    	  <<"********************************* Config End ********************************\n";
	return os;
    }
  private:
    void ParseConfig() {
      if(_file.Open(true) == true) {
        JSONConfig config;
        Core::OptionalType<Core::JSON::Error> error;
        std::cerr<<"Config readng from file: "<<_file.PathName()<<"\n";
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
          if(config.ThreadPoolMaxJobThreshold.IsSet()){
            _threadPoolMaxJobThreshold = config.ThreadPoolMaxJobThreshold.Value();
          } else {
            std::cerr<<"ProxyCategory_MaxThreshold not set so setting default value to 1024\n";
            _threadPoolMaxJobThreshold = 1024;
          }
          //std::cerr<<"File Parsed. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold<< " ThreadPool_MaxJobThreshold: "<<_threadPoolMaxJobThreshold<<'\n';
          std::cerr<<"File Parsed.\n"<<*this; 
        } else {
          std::cerr<<"File Parse Error. Setting all default values.\n"<<*this;
          //std::cerr<<"File Parse Error. Setting all default values. Duration:"<<_duration<<" Freq: "<<_freq<<" ProxyCategory_MaxThreshold: "<<_proxyCategoryMaxThreshold<< " ThreadPool_MaxJobThreshold: "<<_threadPoolMaxJobThreshold<< '\n';
        }
      } else {
        std::cerr<<"Unable to open file. Setting all default values.\n"<<*this;
      }
    }
  private:
    Core::File _file;
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _proxyCategoryMaxThreshold;
    uint32_t _threadPoolMaxJobThreshold;
};

}
}

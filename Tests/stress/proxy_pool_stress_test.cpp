#include<iostream>
#define MODULE_NAME ProxyPoolStressTest
#include <core/core.h>
#include <vector>
#include <TypeTraits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory>
#include <algorithm>

struct TestStruct {
    TestStruct():_a(0){}
    TestStruct(int i):_a(i){}
    ~TestStruct() = default;
    const int getA() const{return _a;}
  private:
    int _a;
};
namespace WPEFramework {

class TrafficGeneratorInterface
{
  public:
    virtual uint32_t getNextTimerValue() = 0;
    virtual uint32_t getObjectCount() = 0;
    virtual const string getName() const = 0;
    virtual ~TrafficGeneratorInterface() = default;
};

//Start of Traffic Generators Implementation
class TriangleTrafficGenerator: public TrafficGeneratorInterface {
  public:
    TriangleTrafficGenerator() = delete;
    TriangleTrafficGenerator(const TriangleTrafficGenerator&) = default;
    TriangleTrafficGenerator& operator=(const TriangleTrafficGenerator&) = delete;
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount): _duration(duration)
                                                                      , _freq(freq)
                                                                      , _maxObjectCount(maxObjectCount)
                                                                      , _elapsedDuration(0)
                                                                      , _stepValue(1)
                                                                      , _waveName("TriangularWave") {
    }
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount, uint32_t stepValue): _duration(duration)
                                                                      , _freq(freq)
                                                                      , _maxObjectCount(maxObjectCount)
                                                                      , _elapsedDuration(0)
                                                                      , _stepValue(stepValue)
                                                                      , _waveName("StepWave") {
    }
    ~TriangleTrafficGenerator() = default;


    uint32_t getNextTimerValue() override {
      _elapsedDuration += _stepValue;
      return (_elapsedDuration > _duration)? 0 : _stepValue;
    }
    uint32_t getObjectCount() override {
      uint32_t period = _duration / (_freq * 2);
      uint32_t objectCount = (_maxObjectCount/period) * (period - abs( (int32_t)((_elapsedDuration % (2 * period)) - period)));
      return objectCount;
    }
    const string getName() const override {
     return _waveName;
    }
  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxObjectCount;
    uint32_t _elapsedDuration;
    uint32_t _stepValue;
    string _waveName;
    std::map<uint64_t, int32_t> _report;
};

class SineTrafficGenerator : public TrafficGeneratorInterface {
  public:
    SineTrafficGenerator() = delete;
    SineTrafficGenerator(const SineTrafficGenerator&) = default;
    SineTrafficGenerator& operator=(const SineTrafficGenerator&) = delete;
    SineTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount): _duration(duration)
                                                                     , _freq(freq)
								     , _maxObjectCount(maxObjectCount)
								     , _elapsedTime(0)
								     , _stepValue(1)
								     , _waveName("SineWave") {
    }
    
    ~SineTrafficGenerator() = default;
    uint32_t getNextTimerValue() override {
      _elapsedTime += _stepValue;
      return (_elapsedTime > _duration) ? 0 : _stepValue;
    }
    uint32_t getObjectCount() override {
      uint32_t objectCount  = ((_maxObjectCount/2) * sin(_elapsedTime * _freq)) + (_maxObjectCount/2);
      return objectCount;
    }
    const string getName() const override {
      return _waveName;
    }
  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxObjectCount;
    uint32_t _elapsedTime;
    uint32_t _stepValue;
    string _waveName;
};
//End of Traffic Generators Implementation

class TestManagerInterface {
  public:
    virtual void NotifyTestComplete(std::map<uint64_t, int32_t>& report) = 0;
    virtual void NotifyTestComplete() = 0;
    virtual ~TestManagerInterface() = default;
};



//Start of CUT (Class Under Test)
class LoadTestInterface {
  public:
    virtual void IncreaseStress(uint32_t fold = 0) = 0;
    virtual void DecreaseStress(uint32_t fold = 0) = 0;
    virtual void MaxStress() = 0;
    virtual void NoStress() = 0;
    virtual void Cleanup() = 0;
    virtual ~LoadTestInterface() = default;
};

class ProxyClassStressTest: public LoadTestInterface {
  public:
    ProxyClassStressTest() = delete;
    ProxyClassStressTest(const ProxyClassStressTest& ) = delete;
    ProxyClassStressTest& operator=(const ProxyClassStressTest&) = delete;
    ProxyClassStressTest(uint32_t maxStress): _list(), _maxStress(maxStress) {

    }
    ~ProxyClassStressTest() {
      if (_list.size() > 0) {
        _list.clear();
      }
    }
    void DecreaseStress(uint32_t fold = 0) {
      uint32_t index = 1;
      do {
        // std::cout<<"Removing element from the list\n";
        if(_list.size() > 0) {
          _list.erase(_list.begin());
        }
        index++;
      }while(index < fold);
      return;
    }
    virtual void IncreaseStress(uint32_t fold = 0) = 0;
    void MaxStress() {
      IncreaseStress(_maxStress);
    }
    void NoStress() {
      _list.clear();
    }
    void Cleanup() {
      return NoStress();
    }
  protected:
    std::list<Core::ProxyType<TestStruct>> _list;
    uint32_t _maxStress;

};

class ProxyPoolStressTest: public ProxyClassStressTest {
  public:
    ProxyPoolStressTest() = delete;
    ProxyPoolStressTest(const ProxyPoolStressTest&) = delete;
    ProxyPoolStressTest& operator=(const ProxyPoolStressTest&) = delete;
    ProxyPoolStressTest(uint32_t maxStress):ProxyClassStressTest(maxStress) {
    }
    ~ProxyPoolStressTest() = default;
    
    void IncreaseStress(uint32_t fold = 0) {
      uint32_t index = 1;
      do {
        // std::cout<<"Adding element to the list\n";
        _list.push_back(_testObject.Element());
        index++;
      }while(index < fold);
      return;
    }
  private:
    static Core::ProxyPoolType<TestStruct> _testObject;
};
Core::ProxyPoolType<TestStruct> ProxyPoolStressTest::_testObject(1024);
class ProxyListStressTest: public ProxyClassStressTest {
  public:
    ProxyListStressTest() = delete;
    ProxyListStressTest(const ProxyListStressTest&) = delete;
    ProxyListStressTest& operator=(const ProxyListStressTest&) = delete;
    ProxyListStressTest(uint32_t maxStress):ProxyClassStressTest(maxStress) {
    }
    ~ProxyListStressTest() = default;
    
    void IncreaseStress(uint32_t fold = 0) {
      uint32_t index = 1;
      do {
        _list.push_back(_testObject.Instance<TestStruct>());
        index++;
      }while(index < fold);
      return;
    }
    
  private:
    static Core::ProxyListType<TestStruct> _testObject;
};
Core::ProxyListType<TestStruct> ProxyListStressTest::_testObject;

class ProxyMapStressTest: public ProxyClassStressTest {
  public:
    ProxyMapStressTest() = delete;
    ProxyMapStressTest(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest& operator=(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest(uint32_t maxStress): ProxyClassStressTest(maxStress) {

    }
    ~ProxyMapStressTest() = default;
    
    void IncreaseStress(uint32_t fold = 0) {
      uint32_t index = 1;
      do {
        WPEFramework::Core::Time now = WPEFramework::Core::Time::Now();
        _list.push_back(_testObject.Instance<TestStruct>(now.Ticks()));
        index++;
      }while(index < fold);
      return;
    }
    
  private:
    static Core::ProxyMapType<uint64_t, TestStruct> _testObject;
};

Core::ProxyMapType<uint64_t, TestStruct> ProxyMapStressTest::_testObject;

class LoadTestClassFactory {
  public:
    static LoadTestInterface* getConcreteTestObject(string objectName, uint32_t maxStress){
      LoadTestInterface* loadTest = nullptr;
      if (objectName.compare("ProxyPool") == 0) {
        loadTest = new ProxyPoolStressTest(maxStress);
      }
      else if (objectName.compare("ProxyList") == 0) {
        loadTest = new ProxyListStressTest(maxStress);
      }
      else if (objectName.compare("ProxyMap") == 0) {
        loadTest = new ProxyMapStressTest(maxStress);
      }
      return loadTest;

    }
};
//End of CUT (Class Under Test)
//Start of Strategies
class TaskExecutionInterface {
  public:
    virtual void ExecuteTest() = 0;
    virtual void CancelTest() = 0;
    virtual void Cleanup() = 0;
    virtual ~TaskExecutionInterface() = default;
};

class TimedTaskExecutionInterface: public TaskExecutionInterface {
  public:
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
    void ExecuteTest() override {
      Core::Time nextTick = Core::Time::Now();
      //TODO: start the timer over here
      _timer.Schedule(nextTick.Ticks(), TimerHandler(this));
      return;
    }

    uint64_t Timed(const uint64_t scheduledTime) {
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
    void Cleanup() override {
      _loadTestInterface->Cleanup();
      return;
    }
    void CancelTest() override {
      _cancelTest = true;
      return;
    }
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
    void ExecuteTest() override {
      
      Core::Time nextTick = Core::Time::Now();
      //TODO: start the timer over here
      _timer.Schedule(nextTick.Ticks(), TimerHandler(this));
      while(_cancelTest != true){
        _loadTestInterface->IncreaseStress();
      }
      _parent->NotifyTestComplete();
    }
    void CancelTest() override {
      _cancelTest = true;
    }
    uint64_t Timed(uint64_t scheduledTime) {
      _cancelTest = true;
      return 0;
    }
    void Cleanup() override {
      _loadTestInterface->Cleanup();
      return;
    }
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

    void ExecuteTest() override {
      while(_cancelTest != true){
        _loadTestInterface->DecreaseStress();
      }
      _parent->NotifyTestComplete();
    }

    void CancelTest() override {
      _cancelTest = true;
    }
    void Cleanup() override {
      _loadTestInterface->Cleanup();
      return;
    }
    uint64_t Timed(uint64_t scheduledTime) {
      _cancelTest = true;
      return 0;
    }
  private:
    TestManagerInterface* _parent;
    LoadTestInterface* _loadTestInterface;
    Core::TimerType<TimerHandler> _timer;
    bool _cancelTest;
    uint32_t _duration;
};

//End of Strategies
class TrafficGeneratorFactory {
  public:
    TrafficGeneratorFactory() = delete;
    ~TrafficGeneratorFactory() {
      while(!_listOfTrafficGens.empty()) {
        delete _listOfTrafficGens.front();
        _listOfTrafficGens.pop_front();
      }
    }
    TrafficGeneratorFactory(uint32_t duration, uint32_t freq, uint32_t maxMem): _duration(duration)
                                                                                , _freq(freq)
                                                                                , _maxMem(maxMem){
      _listOfTrafficGens.emplace_back(new TriangleTrafficGenerator(_duration, _freq, _maxMem));
      _listOfTrafficGens.emplace_back(new TriangleTrafficGenerator(_duration, _freq, _maxMem, 5));
      _listOfTrafficGens.emplace_back(new SineTrafficGenerator(_duration, _freq, _maxMem));

    }
    
    const std::list<TrafficGeneratorInterface*>& getAllTrafficGenerators() const{
      return _listOfTrafficGens;
    }
    const uint32_t getNumberOfTrafficGenerators() const {
      return _listOfTrafficGens.size();
    }

  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxMem;
    std::list<TrafficGeneratorInterface*> _listOfTrafficGens;
};

class TestManager: public TestManagerInterface {
  public:
    TestManager() = delete;
    TestManager(const TestManager&) = delete;
    TestManager& operator=(const TestManager&) = delete;
    ~TestManager() {
      while(!_taskExecutionList.empty()) {
        delete _taskExecutionList.front();
        _taskExecutionList.pop_front();
      }
      delete _loadTestObject;
    }
    TestManager(uint32_t duration, uint32_t freq, uint32_t maxMem, string objTypeName): _loadTestObject(nullptr)
                                                                  , _trafficGeneratorFactory(duration, freq, maxMem)
                                                                  , _taskExecutionList()
                                                                  , _cs(0, _trafficGeneratorFactory.getNumberOfTrafficGenerators())
                                                                  , _timerCount(0){

      _loadTestObject = LoadTestClassFactory::getConcreteTestObject(objTypeName, maxMem);
      if(freq > 0) {

        for (const auto iter: _trafficGeneratorFactory.getAllTrafficGenerators()){
          _taskExecutionList.emplace_back(new LinearTimedTaskExecutor(this, _loadTestObject, iter));
        }
      }
      else {
        _taskExecutionList.emplace_back(new OnlyLoadThreadTaskExecutor(this, _loadTestObject, duration));
        _taskExecutionList.emplace_back(new OnlyUnLoadThreadTaskExecutor(this, _loadTestObject, duration));
      }

    }
    void NotifyTestComplete() override {
      _cs.Unlock();
      return;
    }
    void NotifyTestComplete(std::map<uint64_t, int32_t>& report) override {
      std::cout<<"Test Completed\n";
      for(const auto & elem : report) {
        _masterReport.insert(std::pair<uint64_t, int32_t>(elem.first, elem.second));
      }
      std::cout<<"Timer Completed\n";
      _cs.Unlock();
      return;

    }

    void generateReport() {
      int32_t peak{1};
      int32_t total{0};
      uint32_t noOfRequest = std::count_if(_masterReport.cbegin(), _masterReport.cend(), [](std::pair<uint64_t, int32_t> element){ return element.second > 0? true:false;});
      uint32_t noOfRelease = std::count_if(_masterReport.cbegin(), _masterReport.cend(), [](std::pair<uint64_t, int32_t> element){ return element.second < 0? true:false;});
      auto lambda =  [&peak, &total](std::pair<uint64_t, int32_t> element)mutable{
                        total += element.second;
			if (total > peak)
			{
			  peak = total;
			}};
      std::for_each(_masterReport.begin(), _masterReport.end(), lambda);
      std::cout<<"No. of Operation: "<<_masterReport.size()<<"\n";
      std::cout<<"No. of Request: "<< noOfRequest<<"\n";
      std::cout<<"No. of Release: "<< noOfRelease<<"\n";
      std::cout<<"Peak request: "<< peak<<"\n";


      return;
    }
    
    void WaitForTestToComplete() {
      while(_timerCount > 0) {
        std::cout<<"TimerCount: "<<_timerCount<<" no. Of Traffic Gen:"<<_trafficGeneratorFactory.getNumberOfTrafficGenerators()<<" \n";
        _timerCount--;
        _cs.Lock();
      }
      for(const auto& iter: _taskExecutionList) {
        iter->Cleanup();
      }
      generateReport();
      return;
    }
    void StartTest() {
      for(const auto& iter: _taskExecutionList) {
        iter->ExecuteTest();
        _timerCount++;
      }
      WaitForTestToComplete();
      std::cout<<"All tests completed\n";
    }

    
  private:
    LoadTestInterface* _loadTestObject;
    TrafficGeneratorFactory _trafficGeneratorFactory;
    std::list<TaskExecutionInterface*> _taskExecutionList;
    Core::CountingSemaphore _cs;
    uint32_t _timerCount;
    std::multimap<uint64_t, uint32_t> _masterReport;
};

  void performTest(uint32_t testObj, uint32_t duration, uint32_t freq, uint32_t maxMem){
    string objectName;
    switch(testObj) {
      case 0:
        objectName = "ProxyPool";
        break;
      case 1:
        objectName = "ProxyMap";
        break;
      case 2:
        objectName = "ProxyList";
        break;
    }
    std::cout<<"Testing object: "<<objectName<<'\n';
    TestManager tm(duration, freq, maxMem, objectName);
    tm.StartTest();
    return;
  }
}

int main(int argc, char* argv[]) {
  std::cout<<"Starting stress test v2\n";
  int opt = 0;
  uint32_t duration = 60;
  uint32_t maxMem = 1024;
  uint32_t freq = 1;
  uint32_t objType = 0;
  int retVal = -1;
  while ((opt = getopt(argc, argv, "hd:m:f:t:")) !=-1) {
    switch(opt) {
     case 'd':
	std::cout<<"Duration provided:"<< atoi(optarg)<<" mins\n";
        duration = strtoul(optarg, NULL, 10) * 60;
        break;
     case 'm':
	std::cout<<"MaxMem provided:"<< atoi(optarg)<<"\n";
        maxMem = strtoul(optarg, NULL, 10);
        break;
     case 'f':
	std::cout<<"Freq provided:"<< atoi(optarg)<<"\n";
        freq = strtoul(optarg, NULL, 10);
        break;
     case 't':
         std::cout<<"Selecting Object Type:";
         objType = strtoul(optarg, NULL, 10);
       break;
     case 'h':
     default:
        std::cout<<"Usage: "<<argv[0]<<"-[h] [-d <Duration in mins>] [-m <max mem>] [-f <frequency>] [-t <0-ProxyPool|1-ProxyMap|2-ProxyList>]\n";
        exit(-1);
    }
  }
  if (objType >=0 && objType <= 2){

    WPEFramework::performTest(objType,duration, freq, maxMem);
    retVal = 0;
  } else {
    std::cout<<"Unknown Object Type\n";
  }
  return retVal;
}
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

#include "StressTestAdapters.h"
#include <algorithm>

namespace WPEFramework{
namespace StressTest{

struct TestStruct {
  public:
    TestStruct():_a(-2), _isInitialized(false){}
    explicit TestStruct(int i):_a(i), _isInitialized(false){}
    TestStruct(const TestStruct&) = delete;
    TestStruct& operator=(const TestStruct&) = delete;
    ~TestStruct() = default;
    int getA() const{return _a;}
    uint32_t Initialize() { _isInitialized = true; return 0;}
    void Deinitialize() {_isInitialized = false;}
    bool IsInitialized() {return _isInitialized;}
  private:
    int _a;
    bool _isInitialized;
};

class ProxyPoolObjectStore {
  private:
    ProxyPoolObjectStore():_testObject(1024) {}
  public:
    static ProxyPoolObjectStore& Instance() {
      static ProxyPoolObjectStore os;
      return os;
    }
    inline Core::ProxyType<TestStruct> GetElement() {
      return _testObject.Element();
    }
    inline uint32_t GetQueuedElements() {
      return _testObject.QueuedElements();
    }
    inline uint32_t GetCreatedElements() {
      return _testObject.CreatedElements();
    }
    inline uint32_t GetCount() {
      return _testObject.Count();
    }
    ~ProxyPoolObjectStore() {
      std::cout<<"Proxypool Object store Destructor\n";
    }
  private:
    Core::ProxyPoolType<TestStruct> _testObject;
};

class ProxyListObjectStore{
  private:
    ProxyListObjectStore():_testObject() {
    }
  public:
    static ProxyListObjectStore& Instance() {
        static ProxyListObjectStore plos;
        return plos;
    }
    Core::ProxyType<TestStruct> GetInstance() {
        return _testObject.Instance<TestStruct>();
    }
    uint32_t GetCount() const {
        return _testObject.Count();
    }

    ~ProxyListObjectStore() {
      std::cout<<"ProxyList Count: "<<_testObject.Count()<<'\n';
      _testObject.Clear();
      std::cout<<"ProxyList Count: "<<_testObject.Count()<<'\n';
      std::cout<<"Proxylist Object store Destructor\n";
    }
  private:
    Core::ProxyListType<TestStruct> _testObject;
};

class ProxyMapObjectStore {
  private:
    ProxyMapObjectStore():_testObject() {
    }

  public:
    static ProxyMapObjectStore& Instance() {
        static ProxyMapObjectStore pmos;
        return pmos;
    }
    Core::ProxyType<TestStruct> GetInstance(uint64_t value) {
        return _testObject.Instance<TestStruct>(value);
    }

    uint32_t GetCount() const {
        return _testObject.Count();
    }
    ~ProxyMapObjectStore() {
      std::cout<<"ProxyMap Count: "<<_testObject.Count()<<'\n';
      _testObject.Clear();
      std::cout<<"ProxyMap Count: "<<_testObject.Count()<<'\n';
      std::cout<<"ProxyMap Object store Destructor\n";
    }

  private:
    Core::ProxyMapType<uint64_t, TestStruct> _testObject;

};
template <typename EXECUTOR>
class ProxyClassStressTest:public AdaptorWithExecutor<ProxyClassStressTest<EXECUTOR>, EXECUTOR>, public LoadTestInterface {
  public:
    template<typename... ARGS>
    ProxyClassStressTest(ARGS&&... args): AdaptorWithExecutor<ProxyClassStressTest<EXECUTOR>, EXECUTOR>(*this, std::forward<ARGS>(args)...), _list(), _cs(){
        
    }
    ProxyClassStressTest(const ProxyClassStressTest& ) = delete;
    ProxyClassStressTest& operator=(const ProxyClassStressTest&) = delete;
    ~ProxyClassStressTest() {
      _cs.Lock();
      if (_list.size() > 0) {
        _list.clear();
      }
      _cs.Unlock();
    }
    void DecreaseLoad(uint32_t fold = 0 ) {
      uint32_t index = 1;
        // std::cerr<<"Removing "<<fold<<" elements\n";
      do {
        _cs.Lock();
        if(_list.size() > 0) {
          _list.erase(_list.begin());
        }
        _cs.Unlock();
        index++;
      }while(index < fold);
      std::cerr<<"List Size: "<<_list.size()<<'\n';
      return;
    }
    void MaxLoad() {
      IncreaseLoad(ConfigReader::Instance().ProxyCategoryMaxThreshold());
    }
    void NoLoad() {
      _cs.Lock();
      _list.clear();
      _cs.Unlock();
    //   while(_list.size()>0){
    //       _list.erase(_list.begin());
    //   }
      std::cout<<"_list.size: "<<_list.size()<<'\n';
    }
    void Cleanup() {
      std::cout<<"Cleanup Called\n";
      NoLoad();
    }

    virtual bool Validate() override {

      return true;
    }
  protected:
    std::list<Core::ProxyType<TestStruct>> _list;
    Core::CriticalSection _cs;

};

template<typename EXECUTOR>
class ProxyPoolStressTest: public ProxyClassStressTest<EXECUTOR> {
  public:
    using ProxyClassStressTest<EXECUTOR>::_list;
    using ProxyClassStressTest<EXECUTOR>::_cs;
    template<typename... ARGS>
    ProxyPoolStressTest(string categoryName, ARGS&&... args): ProxyClassStressTest<EXECUTOR>(std::forward<ARGS>(args)...), _categoryName(categoryName){
      CategoryTest::Instance().Register(_categoryName, this);
    }
    ProxyPoolStressTest(const ProxyPoolStressTest&) = delete;
    ProxyPoolStressTest& operator=(const ProxyPoolStressTest&) = delete;
    // ProxyPoolStressTest(uint32_t maxStress):ProxyClassStressTest(maxStress) {
    // }
    ~ProxyPoolStressTest() {
      std::cout<<"ProxyPool Unregistering\n";
      CategoryTest::Instance().UnRegister(_categoryName, this);
      std::cout<<"ProxyPool Unregistering completed\n";
    }

    string GetClassName() const {
        return "ProxyPool";
    }
    
    void IncreaseLoad(uint32_t fold = 0) override{
      // std::cerr<<"Increasing load by "<<fold<<'\n';
      uint32_t index = 1;
      uint32_t uninitCnt = 0;
      uint32_t initCnt = 0;
      do {
        Core::ProxyType<TestStruct> result = ProxyPoolObjectStore::Instance().GetElement();
        if (!result->IsInitialized()) {
            uninitCnt++;
        } else {
            initCnt++;
        }
        _cs.Lock();
        _list.push_back(result);
        _cs.Unlock();
        index++;
      }while(index < fold);
      // std::cerr<<"Requested: "<<fold<<" initialized elem Count: "<<initCnt<<" Uninitialized elem count:" <<uninitCnt<<" ";
      std::cerr<<"List Size: "<<ProxyClassStressTest<EXECUTOR>::_list.size()<<'\n';
      // std::cerr<<"Created Elements: "<<ProxyPoolObjectStore::Instance().GetCreatedElements()<<" Queued Elements: "<<ProxyPoolObjectStore::Instance().GetQueuedElements()<<'\n';
      return;
    }

    virtual bool Validate() override{
        return ((ProxyPoolObjectStore::Instance().GetCreatedElements() - ProxyPoolObjectStore::Instance().GetQueuedElements()) == _list.size());
    }
  private:
    string _categoryName;
};


template<typename EXECUTOR>
class ProxyListStressTest: public ProxyClassStressTest<EXECUTOR> {
  public:
    using ProxyClassStressTest<EXECUTOR>::_list;
    using ProxyClassStressTest<EXECUTOR>::_cs;
    template<typename... ARGS>
    ProxyListStressTest(string categoryName, ARGS&&... args) : ProxyClassStressTest<EXECUTOR>(std::forward<ARGS>(args)...), _categoryName(categoryName) {
      CategoryTest::Instance().Register(_categoryName, this);
    }
    ProxyListStressTest(const ProxyListStressTest&) = delete;
    ProxyListStressTest& operator=(const ProxyListStressTest&) = delete;
    ~ProxyListStressTest() {
      std::cout<<"ProxyList Unregistering\n";
      CategoryTest::Instance().UnRegister(_categoryName, this);
      std::cout<<"ProxyList Unregistering complete\n";
    }
 
    string GetClassName() const {
        return "ProxyList";
    }
       
    void IncreaseLoad(uint32_t fold = 0) override{
      uint32_t index = 1;
      do {
        // Core::ProxyType<TestStruct> result = _testObject.Instance<TestStruct>();
        Core::ProxyType<TestStruct> result = ProxyListObjectStore::Instance().GetInstance();
        if (!result->IsInitialized()){
            std::cerr<<"Received uninitialized element from ProxyList\n";
        }
        _cs.Lock();
        _list.push_back(result);
        _cs.Unlock();
        index++;
      }while(index < fold);
      std::cerr<<"List Size: "<<ProxyClassStressTest<EXECUTOR>::_list.size()<<'\n';
    }
    virtual bool Validate() override{
        return (ProxyListObjectStore::Instance().GetCount() == _list.size());
    }
  private:
    string _categoryName;
};

template<typename EXECUTOR>
class ProxyMapStressTest: public ProxyClassStressTest<EXECUTOR> {
  public:
    using ProxyClassStressTest<EXECUTOR>::_list;
    using ProxyClassStressTest<EXECUTOR>::_cs;
    template<typename... Args>
    ProxyMapStressTest(string categoryName, Args&&... args): ProxyClassStressTest<EXECUTOR>(std::forward<Args>(args)...), _categoryName(categoryName) {
      CategoryTest::Instance().Register(_categoryName, this);
    }
    ProxyMapStressTest(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest& operator=(const ProxyMapStressTest&) = delete;
    ~ProxyMapStressTest() {
      std::cout<<"ProxyMap Unregistering\n";
      CategoryTest::Instance().UnRegister(_categoryName, this);
      std::cout<<"ProxyMap Unregistering complete\n";
    }
    
    string GetClassName() const {
        return "ProxyMap";
    }
    
    void IncreaseLoad(uint32_t fold = 0) override{
      uint32_t index = 1;
      do {
        WPEFramework::Core::Time now = WPEFramework::Core::Time::Now();
        Core::ProxyType<TestStruct> result = ProxyMapObjectStore::Instance().GetInstance(now.Ticks());
        if (!result->IsInitialized()){
            std::cerr<<"Received uninitialized element from ProxyMap\n";
        }
        _cs.Lock();
        _list.push_back(result);
        _cs.Unlock();
        index++;
      }while(index < fold);
      std::cerr<<"List Size: "<<ProxyClassStressTest<EXECUTOR>::_list.size()<<'\n';
    }
    virtual bool Validate() override{
        return (ProxyMapObjectStore::Instance().GetCount() == _list.size());
    }
  private:
    string _categoryName;
};

//TODO: Simplify the templates.
/*
 * ProxyPoolStressTest<SimulatedTraffic> 
 */
// using ConstantTraffic = NonIntervalBasedExecutor<ConstantTrafficGenerator>;

// ProxyPoolStressTest<ConstantTraffic> ppt_nib1("cat1", Direction::INCREASE);
// ProxyPoolStressTest<ConstantTraffic> ppt_nib2("cat1", Direction::DECREASE);
ProxyPoolStressTest<NonIntervalBasedExecutor> ppt_nib("ProxyPoolSimpleLoadTestCategory");
// ProxyPoolStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> ppt_nib("ProxyPoolSimpleLoadTestCategory", 2);
// ProxyPoolStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> ppt_nib("ProxyPoolSimpleLoadTestCategory", 2);
// ProxyPoolStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> ppt_nib1("ProxyPoolSimpleLoadTestCategory",Direction::DECREASE, 2);
// ProxyPoolStressTest<IntervalBasedExecutor<TriangleTrafficGenerator>> ppt("ProxyPoolSimulatedLoadTestCategory");
// ProxyPoolStressTest<IntervalBasedExecutor<TriangleTrafficGenerator>> ppt1("ProxyPoolSimulatedLoadTestCategory",5);
// ProxyPoolStressTest<IntervalBasedExecutor<SineTrafficGenerator>> ppt2("ProxyPoolSimulatedLoadTestCategory");

// ProxyMapStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> pmt_nib("ProxyMapSimpleLoadTestCategory",Direction::INCREASE, 2);
// ProxyMapStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> pmt_nib1("ProxyMapSimpleLoadTestCategory",Direction::DECREASE, 2);
// ProxyMapStressTest<IntervalBasedExecutor<TriangleTrafficGenerator>> pmt("ProxyMapSimulatedLoadTestCategory");
// ProxyMapStressTest<IntervalBasedExecutor<TriangleTrafficGenerator>> pmt1("ProxyMapSimulatedLoadTestCategory", 5);
// ProxyMapStressTest<IntervalBasedExecutor<SineTrafficGenerator>> pmt2("ProxyMapSimulatedLoadTestCategory");

// ProxyListStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> plt_nib("ProxyListSimpleLoadTestCategory",Direction::INCREASE, 2);
// ProxyListStressTest<NonIntervalBasedExecutor<ConstantTrafficGenerator>> plt_nib1("ProxyListSimpleLoadTestCategory",Direction::DECREASE, 2);
// ProxyListStressTest<IntervalBasedExecutor<TriangleTrafficGenerator>> plt("ProxyListSimulatedLoadTestCategory");
// ProxyListStressTest<IntervalBasedExecutor<TriangleTrafficGenerator>> plt1("ProxyListSimulatedLoadTestCategory", 5);
// ProxyListStressTest<IntervalBasedExecutor<SineTrafficGenerator>> plt2("ProxyListSimulatedLoadTestCategory");

} // namespace StressTest
} // namespace WPEFramework

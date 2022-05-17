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
#include "StressTestMacros.h"
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
      _testObject.Clear();
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
      _testObject.Clear();
    }

  private:
    Core::ProxyMapType<uint64_t, TestStruct> _testObject;

};

class ProxyClassStressTest: public StressTestInterface {
  public:
    ProxyClassStressTest(): _list(), _cs(){
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
      do {
        _cs.Lock();
        if(_list.size() > 0) {
          _list.erase(_list.begin());
        }
        _cs.Unlock();
        index++;
      }while(index < fold);
      return;
    }
    void MaxLoad() {
      IncreaseLoad(ConfigReader::Instance().ProxyCategoryMaxThreshold());
    }
    void NoLoad() {
      _cs.Lock();
      _list.clear();
      _cs.Unlock();
    }
    void Cleanup() {
      NoLoad();
    }

    virtual bool Validate() override {

      return true;
    }
  protected:
    std::list<Core::ProxyType<TestStruct>> _list;
    Core::CriticalSection _cs;

};

class ProxyPoolStressTest: public ProxyClassStressTest {
  public:
    ProxyPoolStressTest():ProxyClassStressTest() {
    }
    ~ProxyPoolStressTest() = default;
    void IncreaseLoad(uint32_t fold = 0) override {
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
      return;
    }
    string GetClassName() const {
        return "ProxyPool";
    }
};

class ProxyListStressTest: public ProxyClassStressTest {
  public:
    ProxyListStressTest():ProxyClassStressTest() {
    }
    ~ProxyListStressTest() = default;
    void IncreaseLoad(uint32_t fold = 0) override {
      uint32_t index = 1;
      uint32_t uninitCnt = 0;
      uint32_t initCnt = 0;
      do {
        Core::ProxyType<TestStruct> result = ProxyListObjectStore::Instance().GetInstance();
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
      return;
    }
    string GetClassName() const {
        return "ProxyList";
    }
};


class ProxyMapStressTest: public ProxyClassStressTest {
  public:
    ProxyMapStressTest():ProxyClassStressTest() {
    }
    ~ProxyMapStressTest() = default;
    void IncreaseLoad(uint32_t fold = 0) override {
      uint32_t index = 1;
      uint32_t uninitCnt = 0;
      uint32_t initCnt = 0;
      do {
        WPEFramework::Core::Time now = WPEFramework::Core::Time::Now();
        Core::ProxyType<TestStruct> result = ProxyMapObjectStore::Instance().GetInstance(now.Ticks());
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
      return;
    }
    string GetClassName() const {
        return "ProxyMap";
    }
};

// BEGIN_GROUP("CAT1")
#if 1
LOAD_TEST(ProxyPoolStressTest);
LOAD_TEST_WITH_CUSTOM_THREADS(4, ProxyPoolStressTest);
LOAD_TEST(ProxyMapStressTest);
LOAD_TEST(ProxyListStressTest);
STRESS_TEST(ProxyPoolStressTest);
STRESS_TEST(ProxyMapStressTest);
STRESS_TEST(ProxyListStressTest);
#endif
// END_GROUP
} // namespace StressTest
} // namespace WPEFramework

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

#ifndef _STRESS_TEST_PROXY_TYPES_H_
#define _STRESS_TEST_PROXY_TYPES_H_
#include <core/core.h>
#include <list>
#include "StressTestUtil.h"
namespace WPEFramework {
namespace StressTest
{
struct TestStruct {
    TestStruct():_a(-1){}
    TestStruct(int i):_a(i){}
    ~TestStruct() = default;
    const int getA() const{return _a;}
  private:
    int _a;
};

class ProxyClassStressTest: public LoadTestInterface {
  public:
    ProxyClassStressTest() = delete;
    ProxyClassStressTest(const ProxyClassStressTest& ) = delete;
    ProxyClassStressTest& operator=(const ProxyClassStressTest&) = delete;
    ProxyClassStressTest(uint32_t maxStress): _list(), _maxStress(maxStress) {

    }
    ~ProxyClassStressTest();
    void DecreaseStress(uint32_t fold = 0);
    
    virtual void IncreaseStress(uint32_t fold = 0) = 0;
    void MaxStress();
    void NoStress();
    void Cleanup();
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
    
    void IncreaseStress(uint32_t fold = 0);
  private:
    static Core::ProxyPoolType<TestStruct> _testObject;
};
class ProxyListStressTest: public ProxyClassStressTest {
  public:
    ProxyListStressTest() = delete;
    ProxyListStressTest(const ProxyListStressTest&) = delete;
    ProxyListStressTest& operator=(const ProxyListStressTest&) = delete;
    ProxyListStressTest(uint32_t maxStress):ProxyClassStressTest(maxStress) {
    }
    ~ProxyListStressTest() = default;
    
    void IncreaseStress(uint32_t fold = 0);   
  private:
    static Core::ProxyListType<TestStruct> _testObject;
};
class ProxyMapStressTest: public ProxyClassStressTest {
  public:
    ProxyMapStressTest() = delete;
    ProxyMapStressTest(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest& operator=(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest(uint32_t maxStress): ProxyClassStressTest(maxStress) {

    }
    ~ProxyMapStressTest() = default;
    
    void IncreaseStress(uint32_t fold = 0);    
  private:
    static Core::ProxyMapType<uint64_t, TestStruct> _testObject;
};

} // namespace StressTest

    
} // namespace WPEFramework

#endif

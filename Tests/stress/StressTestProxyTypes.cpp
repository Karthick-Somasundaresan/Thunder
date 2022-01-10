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
#include <iostream>
#define MODULE_NAME ProxyPoolStressTest
#include <core/core.h>
#include <unistd.h>
#include "StressTestManager.h"

namespace WPEFramework {
namespace StressTest {

//Start of CUT (Class Under Test)
struct TestStruct {
    TestStruct():_a(-1){}
    explicit TestStruct(int i):_a(i){}
    TestStruct(const TestStruct&) = delete;
    TestStruct& operator=(const TestStruct&) = delete;
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
    virtual void IncreaseStress(uint32_t fold = 0) = 0;
    ~ProxyClassStressTest() {
      if (_list.size() > 0) {
        _list.clear();
      }
    }
    void DecreaseStress(uint32_t fold = 0 ) {
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
    
    void IncreaseStress(uint32_t fold = 0){
      uint32_t index = 1;
      do {
        _list.push_back(_testObject.Element());
        index++;
      }while(index < fold);
      return;
    }
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
    
    void IncreaseStress(uint32_t fold = 0){
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
class ProxyMapStressTest: public ProxyClassStressTest {
  public:
    ProxyMapStressTest() = delete;
    ProxyMapStressTest(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest& operator=(const ProxyMapStressTest&) = delete;
    ProxyMapStressTest(uint32_t maxStress): ProxyClassStressTest(maxStress) {

    }
    ~ProxyMapStressTest() = default;
    
    void IncreaseStress(uint32_t fold = 0){
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

Core::ProxyPoolType<TestStruct> ProxyPoolStressTest::_testObject(1024);

Core::ProxyListType<TestStruct> ProxyListStressTest::_testObject;

Core::ProxyMapType<uint64_t, TestStruct> ProxyMapStressTest::_testObject;

}
}

void performTest(uint32_t testObj, uint32_t duration, uint32_t freq, uint32_t maxMem){
  WPEFramework::StressTest::TestManager tm(duration, freq, maxMem);
  string objectName;
  switch(testObj) {
    case 0:
    objectName = "ProxyPool";
    tm.SetTestObject(new WPEFramework::StressTest::ProxyPoolStressTest(maxMem));
    break;
    case 1:
    objectName = "ProxyMap";
    tm.SetTestObject(new WPEFramework::StressTest::ProxyMapStressTest(maxMem));
    break;
    case 2:
    objectName = "ProxyList";
    tm.SetTestObject(new WPEFramework::StressTest::ProxyListStressTest(maxMem));
    break;
  }
  std::cout<<"Testing object: "<<objectName<<'\n';
  tm.StartTest();
  return;
}


int main(int argc, char* argv[]) {
  std::cout<<"Starting stress test Modular\n";
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

    performTest(objType,duration, freq, maxMem);
    retVal = 0;
  } else {
    std::cout<<"Unknown Object Type\n";
  }
  return retVal;
}

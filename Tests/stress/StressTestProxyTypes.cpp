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
#include "StressTestProxyTypes.h"
#include "StressTestManager.h"

namespace WPEFramework {
namespace StressTest {

//Start of CUT (Class Under Test)


ProxyClassStressTest::~ProxyClassStressTest() {
  if (_list.size() > 0) {
    _list.clear();
  }
}
void ProxyClassStressTest::DecreaseStress(uint32_t fold) {
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
void ProxyClassStressTest::MaxStress() {
  IncreaseStress(_maxStress);
}
void ProxyClassStressTest::NoStress() {
  _list.clear();
}
void ProxyClassStressTest::Cleanup() {
  return NoStress();
}
 
void ProxyPoolStressTest::IncreaseStress(uint32_t fold) {
  uint32_t index = 1;
  do {
    // std::cout<<"Adding element to the list: fold Value: "<<fold<<" index:"<<index<<"\n";
    _list.push_back(_testObject.Element());
    index++;
  }while(index < fold);
  return;
}
Core::ProxyPoolType<TestStruct> ProxyPoolStressTest::_testObject(1024);

void ProxyListStressTest::IncreaseStress(uint32_t fold) {
  uint32_t index = 1;
  do {
    _list.push_back(_testObject.Instance<TestStruct>());
    index++;
  }while(index < fold);
  return;
}

Core::ProxyListType<TestStruct> ProxyListStressTest::_testObject;

void ProxyMapStressTest::IncreaseStress(uint32_t fold) {
  uint32_t index = 1;
  do {
    WPEFramework::Core::Time now = WPEFramework::Core::Time::Now();
    _list.push_back(_testObject.Instance<TestStruct>(now.Ticks()));
    index++;
  }while(index < fold);
  return;
}
    

Core::ProxyMapType<uint64_t, TestStruct> ProxyMapStressTest::_testObject;

}
}

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
  WPEFramework::StressTest::TestManager tm(duration, freq, maxMem, objectName);
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

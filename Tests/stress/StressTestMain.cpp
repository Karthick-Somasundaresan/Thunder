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

#include <signal.h>
#include "StressTestCommon.h"

int main() {
  char input;
  do{
    std::cout<<"Select an option\n";
    std::cout<<"1. StartTest\n";
    std::cout<<"2. Get No. of test Registered\n";
    std::cout<<"3. Get Registered Test Names\n";
    std::cout<<"4. Get Test Status\n";
    std::cout<<"5. Get Test Running State\n";
    std::cout<<"6. Get ALL Status\n";
    std::cout<<"q. Quit\n";
    std::cin>>input;
    switch(input) {
      case '1':
        WPEFramework::StressTest::TestManager::Instance().StartTest();
        break;
      case '2':
        std::cout<<"**************** No. Of Tests Registered [REPORT-START]*****************\n";
        WPEFramework::StressTest::TestManager::Instance().PrintReport(WPEFramework::StressTest::ReportType::TEST_COUNT);
        std::cout<<"**************** No. Of Tests Registered [REPORT-END]*****************\n";
        break;
      case '3':
        std::cout<<"**************** Tests Registered Names [REPORT-START]*****************\n";
        WPEFramework::StressTest::TestManager::Instance().PrintReport(WPEFramework::StressTest::ReportType::TEST_NAMES);
        std::cout<<"**************** Tests Registered Names [REPORT-END]*****************\n";
        break;
      case '4':
        std::cout<<"**************** Tests Execution status [REPORT-START]*****************\n";
        WPEFramework::StressTest::TestManager::Instance().PrintReport(WPEFramework::StressTest::ReportType::TEST_STATUS);
        std::cout<<"**************** Tests Execution status [REPORT-END]*****************\n";
        break;
      case '5':
        std::cout<<"**************** Tests Execution State [REPORT-START]*****************\n";
        WPEFramework::StressTest::TestManager::Instance().PrintReport(WPEFramework::StressTest::ReportType::TEST_STATE);
        std::cout<<"**************** Tests Execution State [REPORT-END]*****************\n";
        break;
      case '6':
        std::cout<<"**************** ALL [REPORT-START]*****************\n";
        WPEFramework::StressTest::TestManager::Instance().PrintReport(WPEFramework::StressTest::ReportType::ALL);
        std::cout<<"**************** ALL [REPORT-END]*****************\n";
        break;
      case 'q':
        WPEFramework::StressTest::TestManager::Instance().StopExecution();
        break;
      default:
        std::cout<<"Invalid Choice\n";
        break;
    }
  }while(input != 'q');
  return 0;
}
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
void StopTestExecution(int signo) {
    WPEFramework::StressTest::TestManager::Instance().HandleCancelRequest();
}
void InstallSignalHandlers() {
      struct sigaction sa;
  memset(&sa, 0, sizeof(struct sigaction));
  sigemptyset(&sa.sa_mask);
  sa.sa_handler = StopTestExecution;
  sa.sa_flags = 0; 
  sigaction(SIGINT, &sa, nullptr);
}
int main() {
    InstallSignalHandlers();
    WPEFramework::StressTest::TestManager::Instance().PerformTest();
    //TODO: Move away from signals and impl through threads.
    return 0;
}
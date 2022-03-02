# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2020 Metrological
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(WPEFRAMEWORK_STRESS_APP "WPEFramework_stress_test")

add_executable(${WPEFRAMEWORK_STRESS_APP}
   Module.cpp
   StressTestCommon.cpp
   StressTestTrafficShaper.cpp 
   StressTestManager.cpp 
   StressTestProxyCategory.cpp
   StressTestMain.cpp
)

target_compile_definitions(${WPEFRAMEWORK_STRESS_APP}
   PRIVATE BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
)

target_compile_definitions(${WPEFRAMEWORK_STRESS_APP}
   PRIVATE BUILD_DIR=\"${CMAKE_CURRENT_BINARY_DIR}\"
)

target_link_libraries(${WPEFRAMEWORK_STRESS_APP} 
    WPEFrameworkCore
)

install(
    TARGETS ${WPEFRAMEWORK_STRESS_APP} 
    DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

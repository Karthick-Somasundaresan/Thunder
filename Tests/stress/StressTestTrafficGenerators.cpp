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

#define MODULE_NAME ProxyPoolStressTest
#include "StressTestTrafficGenerator.h"

namespace WPEFramework {
namespace StressTest {

//Start of Traffic Generators Implementation
uint32_t TriangleTrafficGenerator::getNextTimerValue() /*override*/{
  _elapsedDuration += _stepValue;
  return (_elapsedDuration > _duration)? 0 : _stepValue;
}
uint32_t TriangleTrafficGenerator::getObjectCount() /*override*/ {
  uint32_t period = _duration / (_freq * 2);
  uint32_t objectCount = (_maxObjectCount/period) * (period - abs( (int32_t)((_elapsedDuration % (2 * period)) - period)));
  return objectCount;
}
const string TriangleTrafficGenerator::getName() const /*override*/ {
 return _waveName;
}

uint32_t SineTrafficGenerator::getNextTimerValue() /*override*/ {
  _elapsedTime += _stepValue;
  return (_elapsedTime > _duration) ? 0 : _stepValue;
}
uint32_t SineTrafficGenerator::getObjectCount() /*override*/ {
  uint32_t objectCount  = ((_maxObjectCount/2) * sin(_elapsedTime * _freq)) + (_maxObjectCount/2);
  return objectCount;
}
const string SineTrafficGenerator::getName() const /*override*/ {
  return _waveName;
}

//End of Traffic Generators Implementation

} //namespace StressTest

} //namespace WPEFramework
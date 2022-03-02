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

#include "StressTestCommon.h"
namespace WPEFramework {
namespace StressTest {

TriangleTrafficGenerator::TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount, uint32_t stepValue, string wavename): _duration(duration)
                                                                      , _freq(freq)
                                                                      , _maxThreshold(maxObjectCount)
                                                                      , _elapsedDuration(0)
                                                                      , _stepValue(stepValue)
                                                                      , _waveName(wavename) {
    }

uint32_t TriangleTrafficGenerator::GetNextTimerValue() {
  _elapsedDuration += _stepValue;
  return ((_elapsedDuration > _duration)? 0 : _stepValue) * 1000;
}
uint32_t TriangleTrafficGenerator::GetValue() {
  if(_stepValue == 1)
    std::cerr<<"Enter From Triangle traffic generator\n";
  else 
    std::cerr<<"Enter From Step traffic generator\n";
  uint32_t period = _duration / (_freq * 2);
  uint32_t value = (_maxThreshold/period) * (period - abs( (int32_t)((_elapsedDuration % (2 * period)) - period)));
  return value;
}
const string TriangleTrafficGenerator::GetName() const {
 return _waveName;
}

uint32_t SineTrafficGenerator::GetNextTimerValue() {
  _elapsedTime += _stepValue;
  return ((_elapsedTime > _duration) ? 0 : _stepValue) * 1000;
}
uint32_t SineTrafficGenerator::GetValue() {
  std::cerr<<"Enter From sine traffic generator\n";
  uint32_t value  = ((_maxThreshold/2) * sin(_elapsedTime * _freq)) + (_maxThreshold/2);
  return value;
}
const string SineTrafficGenerator::GetName() const {
  return _waveName;
}
} // namespace StressTest
} // namespace WPEFramework

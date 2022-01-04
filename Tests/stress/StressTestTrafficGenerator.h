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

#ifndef __STRESS_TEST_TRAFFIC_GENERATOR_H_
#define __STRESS_TEST_TRAFFIC_GENERATOR_H_
#include <core/core.h>
namespace WPEFramework {
namespace StressTest {
struct TrafficGeneratorInterface
{
    virtual uint32_t getNextTimerValue() = 0;
    virtual uint32_t getObjectCount() = 0;
    virtual const string getName() const = 0;
    virtual ~TrafficGeneratorInterface() = default;
};

//Start of Traffic Generators Implementation
class TriangleTrafficGenerator: public TrafficGeneratorInterface {
  public:
    TriangleTrafficGenerator() = delete;
    TriangleTrafficGenerator(const TriangleTrafficGenerator&) = default;
    TriangleTrafficGenerator& operator=(const TriangleTrafficGenerator&) = delete;
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount): _duration(duration)
                                                                      , _freq(freq)
                                                                      , _maxObjectCount(maxObjectCount)
                                                                      , _elapsedDuration(0)
                                                                      , _stepValue(1)
                                                                      , _waveName("TriangularWave") {
    }
    TriangleTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount, uint32_t stepValue): _duration(duration)
                                                                      , _freq(freq)
                                                                      , _maxObjectCount(maxObjectCount)
                                                                      , _elapsedDuration(0)
                                                                      , _stepValue(stepValue)
                                                                      , _waveName("StepWave") {
    }
    ~TriangleTrafficGenerator() = default;


    uint32_t getNextTimerValue() override;
    uint32_t getObjectCount() override;
    const string getName() const override;
  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxObjectCount;
    uint32_t _elapsedDuration;
    uint32_t _stepValue;
    string _waveName;
    std::map<uint64_t, int32_t> _report;
};

class SineTrafficGenerator : public TrafficGeneratorInterface {
  public:
    SineTrafficGenerator() = delete;
    SineTrafficGenerator(const SineTrafficGenerator&) = default;
    SineTrafficGenerator& operator=(const SineTrafficGenerator&) = delete;
    SineTrafficGenerator(uint32_t duration, uint32_t freq, uint32_t maxObjectCount): _duration(duration)
                                                                     , _freq(freq)
								     , _maxObjectCount(maxObjectCount)
								     , _elapsedTime(0)
								     , _stepValue(1)
								     , _waveName("SineWave") {
    }
    
    ~SineTrafficGenerator() = default;
    uint32_t getNextTimerValue() override;
    uint32_t getObjectCount() override ;
    const string getName() const override;
  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxObjectCount;
    uint32_t _elapsedTime;
    uint32_t _stepValue;
    string _waveName;
};
//End of Traffic Generators Implementation

class TrafficGeneratorFactory {
  public:
    TrafficGeneratorFactory() = delete;
    ~TrafficGeneratorFactory() {
      while(!_listOfTrafficGens.empty()) {
        delete _listOfTrafficGens.front();
        _listOfTrafficGens.pop_front();
      }
    }
    TrafficGeneratorFactory(uint32_t duration, uint32_t freq, uint32_t maxMem): _duration(duration)
                                                                                , _freq(freq)
                                                                                , _maxMem(maxMem){
      _listOfTrafficGens.emplace_back(new TriangleTrafficGenerator(_duration, _freq, _maxMem));
      _listOfTrafficGens.emplace_back(new TriangleTrafficGenerator(_duration, _freq, _maxMem, 5));
      _listOfTrafficGens.emplace_back(new SineTrafficGenerator(_duration, _freq, _maxMem));

    }
    
    const std::list<TrafficGeneratorInterface*>& getAllTrafficGenerators() const{
      return _listOfTrafficGens;
    }
    const uint32_t getNumberOfTrafficGenerators() const {
      return _listOfTrafficGens.size();
    }

  private:
    uint32_t _duration;
    uint32_t _freq;
    uint32_t _maxMem;
    std::list<TrafficGeneratorInterface*> _listOfTrafficGens;
};


}
}
#endif
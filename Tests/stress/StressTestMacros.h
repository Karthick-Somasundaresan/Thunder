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

#pragma once
const char* group = "Individual";
#define BEGIN_GROUP(GroupName) \
namespace { \
  const char* group = GroupName;
#define END_GROUP }
#define VARIABLE_NAME_IMPL(x, y) x##y
#define VARIABLE_NAME(ClassName, Number) VARIABLE_NAME_IMPL(ClassName, Number)
#define LOAD_TEST(ClassName,...)\
LoadTestExecutor<ClassName> VARIABLE_NAME(ClassName, __COUNTER__) (3,group, ##__VA_ARGS__)
#define STRESS_TEST(ClassName,...)\
StressTestExecutor<ClassName> VARIABLE_NAME(ClassName, __COUNTER__) (2,group, ##__VA_ARGS__)
#define LOAD_TEST_WITH_CUSTOM_THREADS(ClassName,ThreadCount,...)\
LoadTestExecutor<ClassName> VARIABLE_NAME(ClassName, __COUNTER__) (ThreadCount,group, ##__VA_ARGS__)
#define STRESS_TEST_WITH_CUSTOM_THREADS(ClassName,ThreadCount,...)\
StressTestExecutor<ClassName> VARIABLE_NAME(ClassName, __COUNTER__) (ThreadCount,group, ##__VA_ARGS__)

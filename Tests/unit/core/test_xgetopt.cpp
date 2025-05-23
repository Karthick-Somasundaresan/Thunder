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

#include <gtest/gtest.h>

#ifndef MODULE_NAME
#include "../Module.h"
#endif

#include <core/core.h>

namespace Thunder {
namespace Tests {
namespace Core {

    int argumentCount = 3;
    char* arguments[]= {(char*)"-c", (char*)"-h", (char*)"-b"};

    class ConsoleOptions : public ::Thunder::Core::Options {
        public:
            ConsoleOptions() = delete;

            ConsoleOptions(int argumentCount, TCHAR* arguments[])
                : ::Thunder::Core::Options(argumentCount, arguments, _T("chb"))
            {
                Parse();
            }

            ~ConsoleOptions()
            {
            }

         private:
            virtual void Option(const TCHAR option, VARIABLE_IS_NOT_USED const TCHAR* argument)
            {
                switch (option) {
                case 'c':
                    EXPECT_EQ(option,'c');
                    break;
#ifndef __WIN32__
                case 'b':
                    EXPECT_EQ(option,'b');
                    break;
#endif
                case 'h':
                    EXPECT_EQ(option,'h');
                    break;
                default:
                    printf("default\n");
                    break;
                }
            }
    };

    TEST(test_xgetopt, simple_xgetopt)
    {
        ConsoleOptions consoleOptions(argumentCount,arguments);
    }

} // Core
} // Tests
} // Thunder

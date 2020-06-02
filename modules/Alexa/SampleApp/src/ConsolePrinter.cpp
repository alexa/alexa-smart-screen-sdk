/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include "SampleApp/ConsolePrinter.h"

#include <iostream>
#include <algorithm>

#ifdef UWP_BUILD
#include <windows.h>
#endif

/**
 *  When using pretty print, we pad our strings in the beginning and in the end with the margin representation '#'
 *  and 7 spaces. E.g., if I pass "Hello world!" string, pretty print will look like:
 *  ############################
 *  #       Hello world!       #
 *  ############################
 */
static const size_t PADDING_LENGTH = 8;

namespace alexaSmartScreenSDK {
namespace sampleApp {

using namespace alexaClientSDK;

std::shared_ptr<std::mutex> ConsolePrinter::m_globalMutex = std::make_shared<std::mutex>();

#ifdef UWP_BUILD
std::wstring s2ws(const std::string& sa) {
    std::string& s = sa + "\n";
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
#endif

ConsolePrinter::ConsolePrinter() :
        avsCommon::utils::logger::Logger(avsCommon::utils::logger::Level::UNKNOWN), m_mutex(m_globalMutex) {
}

void ConsolePrinter::simplePrint(const std::string& stringToPrint) {
    auto mutex = m_globalMutex;
    if (!mutex) {
        return;
    }

    std::lock_guard<std::mutex> lock{*mutex};
#ifdef UWP_BUILD
    auto ws = s2ws(stringToPrint);
    OutputDebugString((LPCWSTR)ws.c_str());
#else
    std::cout << stringToPrint << std::endl;
#endif
}

void ConsolePrinter::prettyPrint(std::initializer_list<std::string> lines) {
    size_t maxLength = 0;
    for (auto& line : lines) {
        maxLength = std::max(line.size(), maxLength);
    }

    const std::string line(maxLength + (2 * PADDING_LENGTH), '#');
    std::ostringstream oss;
    oss << line << std::endl;

    // Write each line starting and ending with '#'
    auto padBegin = std::string("#");
    padBegin.append(PADDING_LENGTH - 1, ' ');
    for (auto& line : lines) {
        auto padEnd = std::string("#");
        padEnd.insert(padEnd.begin(), maxLength - line.size() + (PADDING_LENGTH - 1), ' ');
        oss << padBegin << line << padEnd << std::endl;
    }

    oss << line << std::endl;
    simplePrint(oss.str());
}

void ConsolePrinter::prettyPrint(const std::string& stringToPrint) {
    prettyPrint({stringToPrint});
}

void ConsolePrinter::emit(
    avsCommon::utils::logger::Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    std::lock_guard<std::mutex> lock{*m_mutex};

#ifdef UWP_BUILD
    auto formatted = m_logFormatter.format(level, time, threadMoniker, text);
    auto ws = s2ws(formatted);
    OutputDebugString((LPCWSTR)ws.c_str());
#else
    std::cout << m_logFormatter.format(level, time, threadMoniker, text) << std::endl;
#endif
}

}  // namespace sampleApp
}  // namespace alexaSmartScreenSDK

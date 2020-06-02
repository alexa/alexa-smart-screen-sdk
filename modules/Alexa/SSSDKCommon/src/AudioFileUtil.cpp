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

#include <fstream>
#include <vector>

#include "SSSDKCommon/AudioFileUtil.h"

namespace alexaSmartScreenSDK {
namespace sssdkCommon {

std::vector<int16_t> AudioFileUtil::readAudioFromFile(const std::string& fileName, bool& errorOccurred) {
    const int RIFF_HEADER_SIZE = 44;

    std::ifstream inputFile(fileName.c_str(), std::ifstream::binary);
    if (!inputFile.good()) {
        errorOccurred = true;
        return {};
    }
    inputFile.seekg(0, std::ios::end);
    int fileLengthInBytes = inputFile.tellg();
    if (fileLengthInBytes <= RIFF_HEADER_SIZE) {
        errorOccurred = true;
        return {};
    }

    inputFile.seekg(RIFF_HEADER_SIZE, std::ios::beg);

    int numSamples = (fileLengthInBytes - RIFF_HEADER_SIZE) / 2;

    std::vector<int16_t> retVal(numSamples, 0);

    inputFile.read((char*)&retVal[0], numSamples * 2);

    if (inputFile.gcount() != numSamples * 2) {
        errorOccurred = true;

        return {};
    }

    inputFile.close();
    errorOccurred = false;
    return retVal;
}

}  // namespace sssdkCommon
}  // namespace alexaSmartScreenSDK

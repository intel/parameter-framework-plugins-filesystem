/*
* Copyright (c) 2011-2017, Intel Corporation
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
* this list of conditions and the following disclaimer in the documentation and/or
* other materials provided with the distribution.
*
* 3. Neither the name of the copyright holder nor the names of its contributors
* may be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include "ParameterType.h"
#include "MappingContext.h"
#include "FSMappingKeys.h"
#include "InstanceConfigurableElement.h"
#include "FSSubsystemObject.h"

using std::string;
using std::stringstream;
using std::ifstream;

const uint32_t STR_FORMAT_LENGTH = 1024;

#define base CFormattedSubsystemObject

CFSSubsystemObject::CFSSubsystemObject(const string &mappingValue,
                                       CInstanceConfigurableElement *instanceConfigurableElement,
                                       const CMappingContext &context, core::log::Logger &logger)
    : base(instanceConfigurableElement, logger, mappingValue, EFSAmend1,
           (EFSAmendEnd - EFSAmend1 + 1), context),
      _wrongElementTypeErrorOccured(false), _stringFormat(false)
{
    // Get actual element type
    const CParameterType *parameterType =
        static_cast<const CParameterType *>(instanceConfigurableElement->getTypeElement());

    // Retrieve sizes
    _scalarSize = parameterType->getSize();
    _arraySize = instanceConfigurableElement->getFootPrint() / _scalarSize;

    // Amend
    _directoryPath = context.getItem(EFSDirectory) + "/";

    // Handle types
    // Check we are able to handle elements (no exception support, defer the error)
    switch (instanceConfigurableElement->getType()) {
    case CInstanceConfigurableElement::EParameter:
        break;
    case CInstanceConfigurableElement::EStringParameter:
        _stringFormat = true;
        break;
    default:
        _wrongElementTypeErrorOccured = true;
        break;
    }
}
// Sync to/from HW
bool CFSSubsystemObject::accessHW(bool receive, string &error)
{
    // Check parameter type is ok (deferred error, no exceptions available :-()
    if (_wrongElementTypeErrorOccured) {

        error = "Only Parameter and StringParameter types are supported";

        return false;
    }

    return base::accessHW(receive, error);
}

bool CFSSubsystemObject::sendToHW(string &error)
{
    bool success;

    // Warning: file emptied upon opening
    string filePath = _directoryPath + getFormattedMappingValue();
    int fileDesc = open(filePath.c_str(), O_WRONLY | O_TRUNC);

    // Check open is successful
    if (fileDesc == -1) {
        stringstream errorStream;
        errorStream << "Unable to access file " << filePath << " with error " << errno;
        error = errorStream.str();
        return false;
    }

    success = sendToFile(fileDesc, error);
    close(fileDesc);

    return success;
}

bool CFSSubsystemObject::receiveFromHW(string &error)
{
    bool success;
    string filePath = _directoryPath + getFormattedMappingValue();
    ifstream inputFile(filePath.c_str());

    if (inputFile.fail()) {
        stringstream errorStream;
        errorStream << "Unable to open file" << filePath << " with failbit "
                    << (inputFile.rdstate() & ifstream::failbit) << " and badbit "
                    << (inputFile.rdstate() & ifstream::badbit);
        error = errorStream.str();
        return false;
    }

    success = receiveFromFile(inputFile, error);
    inputFile.close();

    return success;
}

bool CFSSubsystemObject::sendToFile(int fileDesc, string &error)
{
    uint32_t index;
    int nbBytes;
    string formatedContent;
    void *blackboardContent = alloca(_scalarSize);
    string filePath = _directoryPath + getFormattedMappingValue();

    for (index = 0; index < _arraySize; index++) {

        // Read Value in BlackBoard
        blackboardRead(blackboardContent, _scalarSize);

        formatedContent = toString(blackboardContent, _scalarSize);

        // WARNING: Current C++ STL implementation of fstream operator '<<' cannot
        //          write a string at once: it instead writes the first character and
        //          then the remainining part of the string.
        //          To support sysfs, we need to be able to write the string at once,
        //          thus we use 'write' API.

        // Try to write the entire string
        nbBytes = write(fileDesc, (const void *)formatedContent.c_str(), formatedContent.size());

        if (nbBytes == -1) {
            // Error when writing
            stringstream errorStream;
            errorStream << "Unable to write element #" << index << "over a total of " << _arraySize
                        << " elements to file " << filePath << " with error " << errno;
            error = errorStream.str();
            return false;
        } else if (nbBytes != (int)formatedContent.size()) {
            // Did not write all characters at once
            stringstream errorStream;
            errorStream << "Unable to write element #" << index << "over a total of " << _arraySize
                        << " elements to file " << filePath << " at once";
            error = errorStream.str();
            return false;
        }
    }

    return true;
}

bool CFSSubsystemObject::receiveFromFile(ifstream &inputFile, string &error)
{
    uint32_t index;
    char formatedContent[STR_FORMAT_LENGTH];
    void *blackboardContent = alloca(_scalarSize);
    string filePath = _directoryPath + getFormattedMappingValue();

    for (index = 0; index < _arraySize; index++) {

        if (!inputFile.good()) {
            stringstream errorStream;
            errorStream << "Unable to read file" << filePath << " with eofbit "
                        << (inputFile.rdstate() & ifstream::eofbit) << ", failbit "
                        << (inputFile.rdstate() & ifstream::failbit) << " and badbit "
                        << (inputFile.rdstate() & ifstream::badbit);
            error = errorStream.str();
            return false;
        }

        inputFile.getline(formatedContent, STR_FORMAT_LENGTH);

        fromString(formatedContent, blackboardContent, _scalarSize);

        // Write Value in Blackboard
        blackboardWrite(blackboardContent, _scalarSize);
    }

    return true;
}

string CFSSubsystemObject::toString(const void *inputData, uint32_t size) const
{
    string outputString;

    if (_stringFormat) {
        // Create a size-long string
        outputString = string((const char *)inputData, size);
    } else {
        stringstream formatedStream;

        // Make sure data is aligned on 32 bits
        int32_t outputInt = 0;

        // For type IntegerParameter, size is the size in bytes
        assert(size <= sizeof(outputInt));

        memcpy((void *)&outputInt, inputData, size);

        // Convert from integer to string
        formatedStream << outputInt;
        outputString = formatedStream.str();
    }

    // Stop at 1st null character to avoid garbage
    outputString = outputString.c_str();

    // Add a new line as a delimitor
    outputString += '\n';

    return outputString;
}

void CFSSubsystemObject::fromString(const string &inputString, void *dest, uint32_t size)
{
    const void *outputData;
    string outputString;
    int32_t outputInt;

    if (_stringFormat) {
        // Content will be truncated if length is greater than 'size - 1'
        outputString = inputString;
        outputString.resize(size - 1);

        // The C string will be at most size-long including null terminating character
        outputData = (const void *)outputString.c_str();
    } else {
        // For type IntegerParameter, size is the size in bytes
        assert(size <= sizeof(outputInt));

        // Extract integer from input string
        stringstream inputStream(inputString);
        inputStream >> outputInt;
        outputData = (const void *)&outputInt;
    }

    memcpy(dest, outputData, size);
}

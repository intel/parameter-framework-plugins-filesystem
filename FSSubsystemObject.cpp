/*
 * INTEL CONFIDENTIAL
 * Copyright  2011 Intel
 * Corporation All Rights Reserved.
 *
 * The source code contained or described herein and all documents related to
 * the source code ("Material") are owned by Intel Corporation or its suppliers
 * or licensors. Title to the Material remains with Intel Corporation or its
 * suppliers and licensors. The Material contains trade secrets and proprietary
 * and confidential information of Intel or its suppliers and licensors. The
 * Material is protected by worldwide copyright and trade secret laws and
 * treaty provisions. No part of the Material may be used, copied, reproduced,
 * modified, published, uploaded, posted, transmitted, distributed, or
 * disclosed in any way without Intels prior express written permission.
 *
 * No license under any patent, copyright, trade secret or other intellectual
 * property right is granted to or conferred upon you by disclosure or delivery
 * of the Materials, either expressly, by implication, inducement, estoppel or
 * otherwise. Any license under such intellectual property rights must be
 * express and approved by Intel in writing.
 *
 * CREATED: 2012-03-29
 * UPDATED: 2012-03-29
 */
#include <fstream>
#include <string>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include "ParameterType.h"
#include "MappingContext.h"
#include "FSMappingKeys.h"
#include "InstanceConfigurableElement.h"
#include "FSSubsystemObject.h"

const uint32_t STR_FORMAT_LENGTH = 1024;

#define base CSubsystemObject

CFSSubsystemObject::CFSSubsystemObject(const string& mappingValue,
                                       CInstanceConfigurableElement* instanceConfigurableElement,
                                       const CMappingContext& context)
    : base(instanceConfigurableElement), _wrongElementTypeErrorOccured(false), _stringFormat(false)
{
    // Get actual element type
    const CParameterType* parameterType
            = static_cast<const CParameterType*>(instanceConfigurableElement->getTypeElement());

    // Retrieve sizes
    _scalarSize = parameterType->getSize();
    _arraySize = instanceConfigurableElement->getFootPrint() / _scalarSize;

    // Amend
    _filePath = context.getItem(EFSDirectory) + "/"
                + formatMappingValue(mappingValue,
                                     EFSAmend1,
                                     (EFSAmendEnd - EFSAmend1 + 1),
                                     context);

    // Handle types
    // Check we are able to handle elements (no exception support, defer the error)
    switch(instanceConfigurableElement->getType()) {
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
bool CFSSubsystemObject::accessHW(bool receive, string& error)
{
    // Check parameter type is ok (deferred error, no exceptions available :-()
    if (_wrongElementTypeErrorOccured) {

        error = "Only Parameter and StringParameter types are supported";

        return false;
    }

    return base::accessHW(receive, error);
}

bool CFSSubsystemObject::sendToHW(string& error)
{
    bool success;

    // Warning: file emptied upon opening
    int fileDesc = open(_filePath.c_str(), O_WRONLY | O_TRUNC);

    // Check open is successful
    if (fileDesc == -1) {
        stringstream errorStream;
        errorStream << "Unable to access file " << _filePath << " with error " << errno;
        error = errorStream.str();
        return false;
    }

    success = sendToFile(fileDesc, error);
    close(fileDesc);

    return success;
}

bool CFSSubsystemObject::receiveFromHW(string& error)
{
    bool success;
    ifstream inputFile(_filePath.c_str());

    if (inputFile.fail()) {
        stringstream errorStream;
        errorStream << "Unable to open file" << _filePath
                    << " with failbit " << (inputFile.rdstate() & ifstream::failbit)
                    << " and badbit " << (inputFile.rdstate() & ifstream::badbit);
        error = errorStream.str();
        return false;
    }

    success = receiveFromFile(inputFile, error);
    inputFile.close();

    return success;
}

bool CFSSubsystemObject::sendToFile(int fileDesc, string& error)
{
    uint32_t index;
    int nbBytes;
    string formatedContent;
    void* blackboardContent = alloca(_scalarSize);

    for (index = 0 ; index < _arraySize ; index++) {

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
            errorStream << "Unable to write element #" << index << "over a total of "
                        << _arraySize << " elements to file " << _filePath
                        << " with error " << errno;
            error = errorStream.str();
            return false;
        } else if (nbBytes != (int)formatedContent.size()) {
            // Did not write all characters at once
            stringstream errorStream;
            errorStream << "Unable to write element #" << index << "over a total of "
                        << _arraySize << " elements to file " << _filePath << " at once";
            error = errorStream.str();
            return false;
        }
    }

    return true;
}

bool CFSSubsystemObject::receiveFromFile(ifstream& inputFile, string& error)
{
    uint32_t index;
    char formatedContent[STR_FORMAT_LENGTH];
    void* blackboardContent = alloca(_scalarSize);

    for (index = 0 ; index < _arraySize ; index++) {

        if(!inputFile.good()) {
            stringstream errorStream;
            errorStream << "Unable to read file" << _filePath
                        << " with eofbit " << (inputFile.rdstate() & ifstream::eofbit)
                        << ", failbit " << (inputFile.rdstate() & ifstream::failbit)
                        << " and badbit " << (inputFile.rdstate() & ifstream::badbit);
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

string CFSSubsystemObject::toString(const void* inputData, uint32_t size) const
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

        memcpy((void*)&outputInt, inputData, size);

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

void CFSSubsystemObject::fromString(const string& inputString, void* dest, uint32_t size)
{
    const void* outputData;
    string outputString;
    int32_t outputInt;

    if (_stringFormat) {
        // Content will be truncated if length is greater than 'size - 1'
        outputString = inputString;
        outputString.resize(size - 1);

        // The C string will be at most size-long including null terminating character
        outputData = (const void*)outputString.c_str();
    } else {
        // For type IntegerParameter, size is the size in bytes
        assert(size <= sizeof(outputInt));

        // Extract integer from input string
        stringstream inputStream(inputString);
        inputStream >> outputInt;
        outputData = (const void*)&outputInt;
    }

    memcpy(dest, outputData, size);
}

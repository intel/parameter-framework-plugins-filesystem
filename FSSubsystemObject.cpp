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
#include <alloca.h>
#include <string.h>
#include <sstream>
#include <stdlib.h>
#include <assert.h>
#include "ParameterType.h"
#include "MappingContext.h"
#include "FSMappingKeys.h"
#include "InstanceConfigurableElement.h"
#include "FSSubsystemObject.h"

#define STR_FORMAT_LENGTH 32

#define base CSubsystemObject

CFSSubsystemObject::CFSSubsystemObject(const string& strMappingValue, CInstanceConfigurableElement* pInstanceConfigurableElement, const CMappingContext& context)
    : base(pInstanceConfigurableElement), _bWrongElementTypeError(false), _bStringFormat(false)
{
    // Get actual element type
    const CParameterType* pParameterType = static_cast<const CParameterType*>(pInstanceConfigurableElement->getTypeElement());

    // Retrieve sizes
    _uiScalarSize = pParameterType->getSize();
    _uiArraySize = pInstanceConfigurableElement->getFootPrint() / _uiScalarSize;

    // Amend
    _strFilePath = context.getItem(EFSDirectory) + "/" + formatMappingValue(strMappingValue, EFSAmend1, (EFSAmendEnd - EFSAmend1 + 1), context);

    // Handle types
    // Check we are able to handle elements (no exception support, defer the error)
    switch(pInstanceConfigurableElement->getType()) {
    case CInstanceConfigurableElement::EParameter:
        break;
    case CInstanceConfigurableElement::EStringParameter:
        _bStringFormat = true;
        break;
    default:
        _bWrongElementTypeError = true;
        break;
    }
}
// Sync to/from HW
bool CFSSubsystemObject::accessHW(bool bReceive, string& strError)
{
    // Check parameter type is ok (deferred error, no exceptions available :-()
    if (_bWrongElementTypeError) {

        strError = "Only Parameter and StringParameter types are supported";

        return false;
    }

    return base::accessHW(bReceive, strError);
}

bool CFSSubsystemObject::sendToHW(string& strError)
{
    // Warning, creation of the file if not existing
    ofstream outputFile;

    outputFile.open(_strFilePath.c_str(), ios_base::trunc);

    // Test on access
    if (!outputFile.is_open()) {

        strError = "Unable to access file: " + _strFilePath;

        return false;
    }

    sendToFile(outputFile);

    outputFile.close();

    return true;
}

bool CFSSubsystemObject::receiveFromHW(string& strError)
{
    (void)strError;

    ifstream inputFile(_strFilePath.c_str());

    if (!inputFile || !inputFile.is_open()) {

        return true;
    }

    receiveFromFile(inputFile);

    inputFile.close();
    return true;
}

void CFSSubsystemObject::sendToFile(ofstream& outputFile)
{
    uint32_t uiIndex;

    void* pvValue = alloca(_uiScalarSize);

    for (uiIndex = 0 ; uiIndex < _uiArraySize ; uiIndex++) {

        // Read Value in BlackBoard
        blackboardRead(pvValue, _uiScalarSize);

        outputFile << toString(pvValue, _uiScalarSize) << endl;
    }
}

void CFSSubsystemObject::receiveFromFile(ifstream& inputFile)
{
    uint32_t uiIndex;

    void* pvValue = alloca(_uiScalarSize);

    for (uiIndex = 0 ; uiIndex < _uiArraySize ; uiIndex++) {

        string strValue;

        inputFile >> strValue;

        fromString(strValue, pvValue, _uiScalarSize);

        // Write Value in Blackboard
        blackboardWrite(pvValue, _uiScalarSize);
    }
}

string CFSSubsystemObject::toString(const void* pvValue, uint32_t uiSize) const
{
    char acFormated[STR_FORMAT_LENGTH];

    if (_bStringFormat) {

        snprintf(acFormated, STR_FORMAT_LENGTH - 1, "%s", (const char*)pvValue);
    } else {
        // Make sure data is align on 32 bits
        int32_t iValue = 0;

        assert(uiSize <= sizeof(iValue));

        memcpy((void*)&iValue, pvValue, uiSize);

        snprintf(acFormated, STR_FORMAT_LENGTH - 1, "%d", iValue);
    }
    acFormated[STR_FORMAT_LENGTH - 1] = '\0';

    return acFormated;
}

void CFSSubsystemObject::fromString(const string& strValue, void* pvValue, uint32_t uiSize)
{
    if (_bStringFormat) {

        strncpy((char*)pvValue, strValue.c_str(), uiSize - 1);

        char* pcValue = (char*)pvValue;

        pcValue[uiSize - 1] = '\0';
    } else {

        int32_t iValue = strtol(strValue.c_str(), NULL, 0);

        assert(uiSize <= sizeof(iValue));

        memcpy(pvValue, (const void*)&iValue, uiSize);
    }
}

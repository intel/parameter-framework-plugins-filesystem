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
#pragma once

#include "SubsystemObject.h"

class CMappingContext;

class CFSSubsystemObject : public CSubsystemObject
{
public:
    CFSSubsystemObject(const string& strMappingValue, CInstanceConfigurableElement* pInstanceConfigurableElement, const CMappingContext& context);

protected:
    // from CSubsystemObject
    // Sync to/from HW
    virtual bool sendToHW(string& strError);
    virtual bool receiveFromHW(string& strError);

private:
    // Sync to/from HW
    virtual bool accessHW(bool bReceive, string& strError);
    void sendToFile(ofstream& outputFile);
    void receiveFromFile(ifstream& inputFile);
    string toString(const void* pvValue, uint32_t uiSize) const;
    void fromString(const string& strValue, void* pvValue, uint32_t uiSize);

protected:
    // Scalar parameter size for elementary access
    uint32_t _uiScalarSize;
    uint32_t _uiArraySize;
    string _strFilePath;
    string _strPath;
    // Delayed error about supported parameter types
    bool _bWrongElementTypeError;
    // Format for reading
    bool _bStringFormat;
};

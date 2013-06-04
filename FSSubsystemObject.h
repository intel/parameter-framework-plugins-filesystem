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
    CFSSubsystemObject(const string& mappingValue,
                       CInstanceConfigurableElement* instanceConfigurableElement,
                       const CMappingContext& context);

protected:
    // from CSubsystemObject
    // Sync to/from HW
    virtual bool sendToHW(string& error);
    virtual bool receiveFromHW(string& error);

private:
    // Sync to/from HW
    virtual bool accessHW(bool receive, string& error);
    /**
      * Read the black board (eventually looping in case of an array),
      * convert the content to ASCII and write it in the file pointed
      * to by iFd file descriptor.
      *
      * @param[in] fileDesc file descriptor of the output file
      * @param[out] error string containing a description of the error
      * in case of failure
      *
      * @return true if success, false if failure.
      *
      * Typical failures are I/O errors.
      *
      * Note that in order to support sysfs, characters have to be
      * written at once. Thus not being able to write the full content
      * in one shot is considered as an error though not related to an
      * I/O error.
      */
    bool sendToFile(int fileDesc, string& error);
    /**
      * Read the file through the ifstream file stream line by line,
      * convert the ASCII value to the black board parameter type and
      * write it in the black board.
      *
      * @param[in] inputFile input file stream
      * @param[out] error string containing a description of the error
      * in case of failure
      *
      * @return true if success, false if failure.
      *
      * Typical failures are I/O errors and EOF reached too early.
      */
    bool receiveFromFile(ifstream& inputFile, string& error);
    /**
      * Convert the input data to a string.
      *
      * Note: the conversion depends on the parameter type:
      * - _stringFormat is true: ASCII->ASCII,
      * - _stringFormat is false: binary->ASCII.
      *
      * @param[in] inputData pointer to data to be converted
      * @param[in] size size of the data
      *
      * @return string resulting of the conversion
      *
      * The output string will be at most size-long. Note that if there
      * is any null character in the string, the remaining part of
      * the string is considered as garbage and thus discarded.
      */
    string toString(const void* inputData, uint32_t size) const;
    /**
      * Extract data from the input string.
      *
      * Note: the extraction depends on the parameter type:
      * - _stringFormat is true: ASCII->ASCII,
      * - _stringFormat is false: ASCII->binary.
      *
      * @param[in] inputString input string data needs to be extracted from
      * @param[out] dest pointer to the memory destination
      * @param[in] size of data to be extracted
      *
      * @return none
      *
      * The output data to be at most size-long. Note that if there
      * is any null character in the input string, the remaining part of
      * the string is considered as garbage and thus discarded.
      */
    void fromString(const string& inputString, void* dest, uint32_t size);

protected:
    // Scalar parameter size for elementary access
    uint32_t _scalarSize;
    uint32_t _arraySize;
    string _filePath;
    // Delayed error about supported parameter types
    bool _wrongElementTypeErrorOccured;
    // Format for reading
    bool _stringFormat;
};

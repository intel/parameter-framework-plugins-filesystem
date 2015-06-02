/*
* Copyright (c) 2011-2015, Intel Corporation
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

#pragma once

#include "FormattedSubsystemObject.h"
#include <string>
#include <fstream>

class CMappingContext;

class CFSSubsystemObject : public CFormattedSubsystemObject
{
public:
    CFSSubsystemObject(const std::string& mappingValue,
                       CInstanceConfigurableElement* instanceConfigurableElement,
                       const CMappingContext& context,
                       core::log::Logger& logger);

protected:
    // from CSubsystemObject
    // Sync to/from HW
    virtual bool sendToHW(std::string& error);
    virtual bool receiveFromHW(std::string& error);

private:
    // Sync to/from HW
    virtual bool accessHW(bool receive, std::string& error);
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
    bool sendToFile(int fileDesc, std::string& error);
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
    bool receiveFromFile(std::ifstream& inputFile, std::string& error);
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
    std::string toString(const void* inputData, uint32_t size) const;
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
    void fromString(const std::string& inputString, void* dest, uint32_t size);

protected:
    // Scalar parameter size for elementary access
    uint32_t _scalarSize;
    uint32_t _arraySize;
    /**
     * String containing the path of the directory containing the file. It corresponds to the
     * 'Directory' information fetched from the XML file.
     */
    std::string _directoryPath;
    // Delayed error about supported parameter types
    bool _wrongElementTypeErrorOccured;
    // Format for reading
    bool _stringFormat;
};

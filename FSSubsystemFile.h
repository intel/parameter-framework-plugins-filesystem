/* <auto_header>
 *
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
 *  AUTHOR: Frederic Boisnard (fredericx.boisnard@intel.com)
 * CREATED: 2012-03-29
 * UPDATED: 2012-03-29
 *
 *
 * </auto_header>
 */
#pragma once

#include "FSSubsystemObject.h"

class CFSSubsystemFile : public CFSSubsystemObject
{
public:
    CFSSubsystemFile(const string& strMappingValue, CInstanceConfigurableElement* pInstanceConfigurableElement, const CMappingContext& context);

private:
    // from CFSSubsystemObject

};

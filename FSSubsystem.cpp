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
#include "FSSubsystem.h"
#include "FSMappingKeys.h"
#include "SubsystemObjectFactory.h"
#include "FSSubsystemObject.h"

#define base CSubsystem

// Implementation
CFSSubsystem::CFSSubsystem(const string& strName) : base(strName)
{
    // Provide mapping keys to upper layer
    addContextMappingKey("Directory");
    addContextMappingKey("Amend1");
    addContextMappingKey("Amend2");
    addContextMappingKey("Amend3");

    // Provide creators to upper layer
    addSubsystemObjectFactory(new TSubsystemObjectFactory<CFSSubsystemObject>("File", 1 << EFSDirectory));
}
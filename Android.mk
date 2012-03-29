LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/parameter-framework-plugins/Fs

LOCAL_SRC_FILES := \
    FSSubsystem.cpp \
    FSSubsystemBuilder.cpp \
    FSSubsystemObject.cpp \
    FSSubsystemFile.cpp

LOCAL_CFLAGS := -D_POSIX_SOURCE

LOCAL_MODULE := libfs-subsystem

LOCAL_MODULE_TAGS := optional

TARGET_ERROR_FLAGS += -Wno-non-virtual-dtor

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../parameter \
    $(LOCAL_PATH)/../xmlserializer/

LOCAL_C_INCLUDES += \
        external/stlport/stlport/ \
        bionic/libstdc++ \
        bionic/ \
        vendor/intel/hardware/include

LOCAL_C_INCLUDES +=

LOCAL_SHARED_LIBRARIES := libstlport libparameter libxmlserializer libcutils

LOCAL_LDLIBS +=
LOCAL_LDFLAGS +=

include $(BUILD_SHARED_LIBRARY)



LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog
LOCAL_MODULE := helper
LOCAL_SRC_FILES := helper.c
LOCAL_CFLAGS += -fPIE
LOCAL_LDFLAGS += -fPIE -pie
include $(BUILD_EXECUTABLE)

# include $(CLEAR_VARS)
# LOCAL_LDLIBS := -llog
# LOCAL_MODULE := ping
# LOCAL_SRC_FILES := ping.cpp
# include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_LDLIBS := -llog -ljnigraphics
LOCAL_MODULE := pinger
LOCAL_SRC_FILES := pinger.cpp ping.cpp os_generic.cpp CNFGFunctions.c CNFGDriver.cpp
# LOCAL_STATIC_LIBRARIES := ping
# LOCAL_CPP_FEATURES := rtti exceptions
include $(BUILD_SHARED_LIBRARY)

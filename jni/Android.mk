LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := imgui
LOCAL_SRC_FILES := \
    imgui/imgui.cpp \
    imgui/imgui_draw.cpp \
    imgui/imgui_widgets.cpp \
    imgui/imgui_tables.cpp \
    imgui/imgui_impl_android.cpp \
    imgui/imgui_impl_opengl3.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/imgui
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := animalcompanysickassmenu
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/imgui
LOCAL_STATIC_LIBRARIES := imgui
LOCAL_LDLIBS := -llog -ldl -landroid -lEGL -lGLESv2
include $(BUILD_SHARED_LIBRARY)
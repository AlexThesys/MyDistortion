cmake_minimum_required(VERSION 3.14.0)

set(vst3sdk_SOURCE_DIR C:/Users/alexs/VST_SDK/VST3_SDK)
if(NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif()

project(MyDistortion)
  
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")

add_subdirectory(${vst3sdk_SOURCE_DIR} ${PROJECT_BINARY_DIR}/vst3sdk)
smtg_enable_vst3_sdk()

smtg_add_vst3plugin(MyDistortion     
    source/version.h
    source/myplugincids.h
    source/mypluginprocessor.h
    source/mypluginprocessor.cpp
    source/myplugincontroller.h
    source/myplugincontroller.cpp
    source/mypluginentry.cpp
    source/waveshaper.cpp
    source/constants.h
)

#- VSTGUI Wanted ----
if(SMTG_ADD_VSTGUI)
    target_sources(MyDistortion
        PRIVATE
            resource/myplugineditor.uidesc
    )
    target_link_libraries(MyDistortion
        PRIVATE
            vstgui_support
    )
    smtg_add_vst3_resource(MyDistortion "resource/myplugineditor.uidesc")
endif()
# -------------------

smtg_add_vst3_snapshot(MyDistortion resource/28EC1380478D509F9156256B3BC3E1C9_snapshot.png)
smtg_add_vst3_snapshot(MyDistortion resource/28EC1380478D509F9156256B3BC3E1C9_snapshot_2.0x.png)

target_link_libraries(MyDistortion
    PRIVATE
        sdk
)

if(SMTG_MAC)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    smtg_set_bundle(MyDistortion
		BUNDLE_IDENTIFIER com.alexsuprunov.MyDistortion
		INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS
    )
    smtg_set_debug_executable(MyDistortion
        "/Applications/VST3PluginTestHost.app"
        "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
    )
elseif(SMTG_WIN)
    target_sources(MyDistortion PRIVATE 
        resource/win32resource.rc
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT MyDistortion)
    
        smtg_set_debug_executable(MyDistortion
            "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
            "--pluginfolder \"$(OutDir)/\""
        )
    endif()
endif()

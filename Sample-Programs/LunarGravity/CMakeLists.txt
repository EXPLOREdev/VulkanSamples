if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR -DWIN32_LEAN_AND_MEAN)
    set(DisplayServer Win32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
    add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if (NOT DEMOS_WSI_SELECTION)
        set(DEMOS_WSI_SELECTION "XCB")
    endif()

    if (DEMOS_WSI_SELECTION STREQUAL "XCB")
        if (NOT BUILD_WSI_XCB_SUPPORT)
            message( FATAL_ERROR "Selected XCB for demos build but not building Xcb support" )
        endif()
        include_directories(${XCB_INCLUDE_DIRS})
        link_libraries(${XCB_LIBRARIES})
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    elseif(DEMOS_WSI_SELECTION STREQUAL "XLIB")
        if (NOT BUILD_WSI_XLIB_SUPPORT)
            message( FATAL_ERROR "Selected XLIB for demos build but not building Xlib support" )
        endif()
        include_directories(${X11_INCLUDE_DIR})
        link_libraries(${X11_LIBRARIES})
        add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
    elseif(DEMOS_WSI_SELECTION STREQUAL "WAYLAND")
        if (NOT BUILD_WSI_WAYLAND_SUPPORT)
            message( FATAL_ERROR "Selected Wayland for demos build but not building Wayland support" )
        endif()
        include_directories(${WAYLAND_CLIENT_INCLUDE_DIR})
        link_libraries(${WAYLAND_CLIENT_LIBRARIES})
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
    elseif(DEMOS_WSI_SELECTION STREQUAL "MIR")
        if (NOT BUILD_WSI_MIR_SUPPORT)
            message( FATAL_ERROR "Selected MIR for demos build but not building Mir support" )
        endif()
        add_definitions(-DVK_USE_PLATFORM_MIR_KHR)
        include_directories(${MIR_INCLUDE_DIR})
        # TODO - Add Mir support
    elseif(DEMOS_WSI_SELECTION STREQUAL "DISPLAY")
        add_definitions(-DVK_USE_PLATFORM_DISPLAY_KHR)
    else()
        message( FATAL_ERROR "Unrecognized value for DEMOS_WSI_SELECTION: ${DEMOS_WSI_SELECTION}" )
    endif()

    link_libraries(${API_LOWERCASE} m)
else()
    message(FATAL_ERROR "Unsupported Platform!")
endif()

file(GLOB RESOURCES
  "${PROJECT_SOURCE_DIR}/Sample-Programs/LunarGravity/resources"
)

file(COPY ${RESOURCES} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

if(WIN32)
  set (LIBRARIES "${API_LOWERCASE}-${MAJOR}")
elseif(UNIX)
  set (LIBRARIES "${API_LOWERCASE}")
else()
endif()

# Define macro used for building shader SPIR-V output
macro(generate_gravity_spv input output)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/${output}
        COMMAND ${GLSLANG_VALIDATOR} -s -V -o ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/${output} ${PROJECT_SOURCE_DIR}/Sample-Programs/LunarGravity/resources/shaders/${input}
        DEPENDS ${GLSLANG_VALIDATOR} ${PROJECT_SOURCE_DIR}/Sample-Programs/LunarGravity/resources/shaders/${input}
        COMMENT "Converting ${PROJECT_SOURCE_DIR}/Sample-Programs/LunarGravity/resources/shaders/${input} to ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/${output}"
    )
endmacro()

if(WIN32)
    # Use static MSVCRT libraries
    foreach(configuration in CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_RELWITHDEBINFO
                             CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_RELWITHDEBINFO)
        if(${configuration} MATCHES "/MD")
            string(REGEX REPLACE "/MD" "/MT" ${configuration} "${${configuration}}")
        endif()
    endforeach()
endif()

generate_gravity_spv(simple_single_tex.vert simple_single_tex-vs.spv)
generate_gravity_spv(simple_single_tex.frag simple_single_tex-fs.spv)

if(WIN32)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES")
endif()

add_executable(LunarGravity
                ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/simple_single_tex-vs.spv
                ${CMAKE_CURRENT_BINARY_DIR}/resources/shaders/simple_single_tex-fs.spv
                lglogger.cpp
                lglogger.hpp
                lgwindow.cpp
                lgwindow.hpp
                lggfxengine.cpp
                lggfxengine.hpp
                main.cpp)
target_link_libraries(LunarGravity ${LIBRARIES})

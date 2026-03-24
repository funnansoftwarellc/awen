project(awen)

# Android NativeActivity requires a shared library (.so), not an executable
if(ANDROID)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_executable(${PROJECT_NAME})
endif()

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        awen-graphics
)

if(ANDROID)
    # ANativeActivity_onCreate is in raylib's static archive but called by the
    # Android runtime at load time (not via a link reference), so the linker
    # would otherwise strip it as unreferenced. Force it to be kept.
    target_link_options(${PROJECT_NAME}
        PRIVATE
            "-Wl,-u,ANativeActivity_onCreate"
    )
endif()

if(MSVC)
    target_link_options(${PROJECT_NAME}
        PRIVATE
            $<$<CONFIG:Debug>:/NODEFAULTLIB:MSVCRT>
    )

    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            winmm
    )
endif()

install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)
cmake_minimum_required (VERSION 3.13)
project (engine)

add_executable(engine)
add_subdirectory(src)
include_directories(. src)

if (UNIX)

if (DBG)
target_link_options(engine PRIVATE -fuse-ld=lld -lSDL2
    -fsanitize=address -fsanitize=undefined -fsanitize=implicit-conversion
)
target_compile_options(engine PRIVATE -g3 -fno-omit-frame-pointer -O0 -march=native -std=c++17 -fno-exceptions -fno-rtti -Wpedantic -Wall -Wextra
    -fsanitize=address -fsanitize=undefined -fsanitize=implicit-conversion
)
endif (DBG)

if (OPT)
target_compile_options(engine PRIVATE -std=c++17 -fno-exceptions -Ofast -march=native -flto -ffunction-sections -fdata-sections) 
target_link_options(engine PRIVATE -fuse-ld=lld -lSDL2 -flto -Wl,-O3 -Wl,--gc-sections -Wl,--strip-all -Wl,--icf=all -Wl,--lto-O3)
endif (OPT)

endif (UNIX)



if (WIN32)
add_custom_command(TARGET engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy Release/engine.exe ${CMAKE_SOURCE_DIR})
target_link_libraries(engine ${CMAKE_SOURCE_DIR}/SDL2.lib)
target_compile_options(engine PRIVATE /std:c++17 /GR- /EHs-c- /Ox /GL)
target_link_options(engine PRIVATE /LTCG /SUBSYSTEM:windows /ENTRY:mainCRTStartup)
#target_compile_options(engine PRIVATE /std:c++17 /GR- /EHs-c- /GL /Z7 /ZI /Zi /Zo /EHsc)
#target_link_options(engine PRIVATE /SUBSYSTEM:windows /ENTRY:mainCRTStartup /DEBUG)
endif (WIN32)

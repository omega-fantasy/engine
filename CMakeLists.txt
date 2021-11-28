cmake_minimum_required (VERSION 3.13)
project (engine)

add_executable(engine)
add_subdirectory(src)
include_directories(. src)

if (UNIX)

set(CMAKE_CXX_CPPCHECK 
    cppcheck
    -I ..;
    -I .;
    --enable=warning;
    --platform=native;
    --inconclusive;
    --max-configs=1;
    )

set(CMAKE_CXX_CLANG_TIDY 
    clang-tidy;
    -extra-arg=-I..;
    -extra-arg=-I.;
    -checks='alpha*,clang-analyzer-*,bugprone-*,misc-*,-bugprone-integer-division,-bugprone-narrowing-conversions,-misc-definitions-in-headers,-misc-unused-using-decls,-clang-analyzer-cplusplus.NewDeleteLeaks,-misc-non-private-member-variables-in-classes,-misc-no-recursion,-bugprone-macro-parentheses,-bugprone-unhandled-self-assignment,-bugprone-reserved-identifier;
    -extra-arg=-std=c++17;
    -header-filter=.*;)

if (DBG)

target_link_options(engine PRIVATE 
    -fuse-ld=lld
    -lSDL2 
    -lSDL2_ttf
    -lpthread
    #-fsanitize=thread
    #-fsanitize=address 
    #-fsanitize=undefined
    #-fsanitize=bounds
    #-fsanitize=leak 
    #-fsanitize=float-divide-by-zero 
    #-fsanitize=float-cast-overflow 
    #-fsanitize=integer
    )

target_compile_options(engine PRIVATE
    -g3 
    -fno-omit-frame-pointer
    -O0
    -std=c++17
    -fno-exceptions
    -fno-rtti
    -Wpedantic
    -Wall
    -Wextra
    -Wshadow
    -Wnull-dereference
    -Wunreachable-code
    -Wuninitialized
    -Wunused
    -Waddress
    -Warray-bounds
    -Wdiv-by-zero
    -Wenum-compare
    -Wimplicit-fallthrough
    -Wvla
    #-fsanitize=thread
    #-fsanitize=address 
    #-fsanitize=undefined
    #-fsanitize=bounds
    #-fsanitize=leak 
    #-fsanitize=float-divide-by-zero 
    #-fsanitize=float-cast-overflow 
    #-fsanitize=integer
)

endif (DBG)

if (OPT)

target_compile_options(engine PRIVATE
    -std=c++17
    -fno-exceptions
    -fno-rtti
    -Ofast
    -march=native
    -flto
    #-fstrict-vtable-pointers
    #-fwhole-program-vtables
    #-fforce-emit-vtables
    -ffunction-sections
    -fdata-sections
    ) 

target_link_options(engine PRIVATE 
    -fuse-ld=gold
    -lSDL2 
    -flto
    -lpthread
    -Wl,-O3
    -Wl,--gc-sections
    -Wl,--strip-all 
    -Wl,--icf=all  
    #-Wl,--lto-O3
    )

endif (OPT)

endif (UNIX)



if (WIN32)
add_custom_command(TARGET engine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy Release/engine.exe ${CMAKE_SOURCE_DIR})
target_link_libraries(engine ${CMAKE_SOURCE_DIR}/SDL2.lib)
target_compile_options(engine PRIVATE 
   /std:c++17
   /GR-
   /EHs-c-
   /Ox
   /GL
   #/Z7
   #/ZI
   #/Zi
   #/Zo
   #/EHsc
)
target_link_options(engine PRIVATE 
   /LTCG
   /SUBSYSTEM:windows
   /ENTRY:mainCRTStartup
   #/DEBUG
)
endif (WIN32)

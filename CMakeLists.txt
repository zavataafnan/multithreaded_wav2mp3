cmake_minimum_required(VERSION 3.10)

project(wavetomp3convertor)


set(sources src/Main.cpp src/Task.cpp src/datathread.cpp)
set(headers src/Task.h src/datathread.h)




if(WIN32)

include_directories("winlib/pthreads-win/include")
include_directories("winlib/lame/include")
include_directories("winlib/sndfile/include")


file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Debug")
file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Release")


if( CMAKE_SIZEOF_VOID_P EQUAL 8 )

link_directories("winlib/pthreads-win/lib/x64/")
link_directories("winlib/sndfile/lib/x64/")
link_directories("winlib/lame/lib/x64/")

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/pthreads-win/dll/x64/pthreadVC2.dll 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/pthreads-win/dll/x64/pthreadVC2.dll 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/sndfile/lib/x64/libsndfile-1.dll
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/sndfile/lib/x64/libsndfile-1.dll 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)


else()
link_directories("winlib/pthreads-win/lib/x86/")
link_directories("winlib/sndfile/lib/x86/")
link_directories("winlib/lame/lib/x86/")


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/pthreads-win/dll/x86/pthreadVC2.dll 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/pthreads-win/dll/x86/pthreadVC2.dll 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/sndfile/lib/x86/libsndfile-1.dll
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug/)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/winlib/sndfile/lib/x86/libsndfile-1.dll 
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release/)



endif()



else()

link_directories("linuxlib/sndfile/lib/")
link_directories("linuxlib/lame/lib/")

include_directories("linuxlib/lame/include")
include_directories("linuxlib/sndfile/include")

  set(THREADS_PREFER_PTHREAD_FLAG ON)
  find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


endif()



add_executable(${PROJECT_NAME} ${sources} ${headers})


if(WIN32)

target_link_libraries(${PROJECT_NAME} libmp3lame-static.lib)
target_link_libraries(${PROJECT_NAME} pthreadVC2.lib)
target_link_libraries(${PROJECT_NAME} libsndfile-1.lib)

set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 17)




else()

target_link_libraries(${PROJECT_NAME} libmp3lame.a)
target_link_libraries(${PROJECT_NAME} sndfile.a)

target_link_libraries(${PROJECT_NAME} Threads::Threads stdc++fs)



endif()








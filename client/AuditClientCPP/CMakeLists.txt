cmake_minimum_required(VERSION 3.0)
project(AuditClientCPP)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

find_package(Boost 1.66.0 REQUIRED COMPONENTS system thread)

if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

# Added because of an error to do with beast or asio error codes not working
add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY) 

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

set(auditclientcpp_SRC
  src/main.cpp
  src/auditclientcpp.cpp
  src/app.cpp
  src/httpclient.cpp
)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tell CMake to create the helloworld executable
add_executable(auditclientcpp ${auditclientcpp_SRC})

# Target C++17
set_property(TARGET auditclientcpp PROPERTY CXX_STANDARD 17)

# Use the Widgets module from Qt 5.
target_link_libraries(auditclientcpp Qt5::Widgets)
if(Boost_FOUND)
    target_link_libraries(auditclientcpp ${Boost_LIBRARIES})
    target_link_libraries(auditclientcpp Threads::Threads)
endif()

# Install the executable
install(TARGETS auditclientcpp DESTINATION bin)

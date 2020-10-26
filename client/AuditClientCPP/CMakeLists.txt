cmake_minimum_required(VERSION 3.0)
project(AuditClientCPP)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to create code from Qt designer ui files
set(CMAKE_AUTOUIC ON)

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

set_property(TARGET auditclientcpp PROPERTY CXX_STANDARD 17)

# Use the Widgets module from Qt 5.
target_link_libraries(auditclientcpp Qt5::Widgets)

# Install the executable
install(TARGETS auditclientcpp DESTINATION bin)


add_definitions(-DBOOST_ERROR_CODE_HEADER_ONLY)

cmake_minimum_required(VERSION 3.8)
project(homegear_mbus)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/PhysicalInterfaces/IMBusInterface.cpp
        src/PhysicalInterfaces/IMBusInterface.h
        src/PhysicalInterfaces/Amber.cpp
        src/PhysicalInterfaces/Amber.h
        src/Factory.cpp
        src/Factory.h
        src/GD.cpp
        src/GD.h
        src/Interfaces.cpp
        src/Interfaces.h
        src/MyCentral.cpp
        src/MyCentral.h
        src/MyFamily.cpp
        src/MyFamily.h
        src/MyPacket.cpp
        src/MyPacket.h
        src/MyPeer.cpp
        src/MyPeer.h
        config.h)

add_custom_target(homegear COMMAND ../../makeAll.sh SOURCES ${SOURCE_FILES})

add_library(homegear_mbus ${SOURCE_FILES})
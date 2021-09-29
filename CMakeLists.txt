cmake_minimum_required(VERSION 3.8)
project(homegear_mbus)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        src/Crc16.cpp
        src/Crc16.h
        src/DescriptionCreator.cpp
        src/DescriptionCreator.h
        src/PhysicalInterfaces/IMbusInterface.cpp
        src/PhysicalInterfaces/IMbusInterface.h
        src/PhysicalInterfaces/Amber.cpp
        src/PhysicalInterfaces/Amber.h
        src/PhysicalInterfaces/Hgdc.cpp
        src/PhysicalInterfaces/Hgdc.h
        src/Factory.cpp
        src/Factory.h
        src/GD.cpp
        src/GD.h
        src/Interfaces.cpp
        src/Interfaces.h
        src/MbusCentral.cpp
        src/MbusCentral.h
        src/Mbus.cpp
        src/Mbus.h
        src/MbusPacket.cpp
        src/MbusPacket.h
        src/MbusPeer.cpp
        src/MbusPeer.h
        src/VifConverter.cpp
        src/VifConverter.h)

add_custom_target(homegear COMMAND ../../devscripts/makeAll.sh SOURCES ${SOURCE_FILES})

add_library(homegear_mbus ${SOURCE_FILES})
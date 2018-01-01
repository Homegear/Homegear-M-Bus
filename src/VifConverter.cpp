/* Copyright 2013-2017 Homegear UG (haftungsbeschränkt) */

#include "VifConverter.h"
#include "GD.h"

namespace MyFamily
{

PVariable VifConverter::getVariable(uint8_t type, std::vector<uint8_t>& vifs, const std::vector<uint8_t>& value)
{
    try
    {
        uint8_t vif = vifs.at(0);
        if(vif == 0x6C)
        {
            //E. g.: 332C for 19th December, 2017
            int32_t year = ((value.at(1) & 0xF0) >> 1) | (value.at(0) >> 5);
            uint8_t month = value.at(1) & 0x0F;
            uint8_t day = value.at(0) & 0x1F;
            if(year == 0 || month == 0 || day == 0) return std::make_shared<Variable>(0);
            year += (year >= 70 ? 1900 : 2000);
            std::tm t = {};
            std::stringstream stringStream;
            stringStream << std::setfill('0') << std::setw(4) << std::to_string(year) << "-" << std::setw(2) << std::to_string(month) << "-" << std::setw(2) << std::to_string(day);
            stringStream >> std::get_time(&t, "%Y-%m-%d");
            if(stringStream.fail()) return std::make_shared<Variable>(0);
            std::time_t time = std::mktime(&t);
            return std::make_shared<Variable>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::from_time_t(time).time_since_epoch()).count());
        }
        else if(vif == 0x6D)
        {
            //E. g.: 0631332C for 19th December, 2017
            int32_t year = ((value.at(3) & 0xF0) >> 1) | (value.at(2) >> 5);
            uint8_t month = value.at(3) & 0x0F;
            uint8_t day = value.at(2) & 0x1F;
            if(year == 0 || month == 0 || day == 0) return std::make_shared<Variable>(0);
            year += (year >= 70 ? 1900 : 2000);
            uint8_t hour = value.at(1) & 0x1F;
            uint8_t minute = value.at(0) & 0x3F;
            if(value.at(0) & 0x80) //Time invalid
            {
                hour = 0;
                minute = 0;
            }

            std::tm t = {};
            std::stringstream stringStream;
            stringStream << std::setfill('0') << std::setw(4) << std::to_string(year) << "-" << std::setw(2) << std::to_string(month) << "-" << std::setw(2) << std::to_string(day) << " " << std::setw(2) << std::to_string(hour) << ":" << std::setw(2) << std::to_string(minute);
            stringStream >> std::get_time(&t, "%Y-%m-%d %H:%M");
            if(stringStream.fail()) return std::make_shared<Variable>();
            std::time_t time = std::mktime(&t);
            return std::make_shared<Variable>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::from_time_t(time).time_since_epoch()).count());
        }
        else if(vif == 0x7C)
        {
            if(value.size() == 1) return std::make_shared<Variable>(std::string());
            return std::make_shared<Variable>(std::string((char*)value.data() + 1, value.size() - 1));
        }

        if(type == 0)
        {
            return std::make_shared<Variable>();
        }
        else if(type == 1)
        {
            return std::make_shared<Variable>(value.back());
        }
        else if(type == 2)
        {
            return std::make_shared<Variable>((((int32_t)value.at(1)) << 8) | value.at(0));
        }
        else if(type == 3)
        {
            return std::make_shared<Variable>((((int32_t)value.at(2)) << 16) | (((int32_t)value.at(1)) << 8) | value.at(0));
        }
        else if(type == 4)
        {
            return std::make_shared<Variable>((((int32_t)value.at(3)) << 24) | (((int32_t)value.at(2)) << 16) | (((int32_t)value.at(1)) << 8) | value.at(0));
        }
        else if(type == 5)
        {
            return std::make_shared<Variable>(BaseLib::Math::getFloatFromIeee754Binary32((((int32_t)value.at(3)) << 24) | (((int32_t)value.at(2)) << 16) | (((int32_t)value.at(1)) << 8) | value.at(0)));
        }
        else if(type == 6)
        {
            return std::make_shared<Variable>((((int64_t)value.at(5)) << 40) | (((int64_t)value.at(4)) << 32) | (((int64_t)value.at(3)) << 24) | (((int64_t)value.at(2)) << 16) | (((int64_t)value.at(1)) << 8) | value.at(0));
        }
        else if(type == 7)
        {
            return std::make_shared<Variable>((((int64_t)value.at(7)) << 56) | (((int64_t)value.at(6)) << 48) | (((int64_t)value.at(5)) << 40) | (((int64_t)value.at(4)) << 32) | (((int64_t)value.at(3)) << 24) | (((int64_t)value.at(2)) << 16) | (((int64_t)value.at(1)) << 8) | value.at(0));
        }
        else if(type == 9)
        {
            return std::make_shared<Variable>((((int32_t)(value.at(0) >> 4)) * 10) + (value.at(0) & 0x0F));
        }
        else if(type == 10)
        {
            return std::make_shared<Variable>((((int32_t)(value.at(1) >> 4)) * 1000) + (((int32_t)(value.at(1) & 0x0F)) * 100) + (((int32_t)(value.at(0) >> 4)) * 10) + (value.at(0) & 0x0F));
        }
        else if(type == 11)
        {
            return std::make_shared<Variable>((((int32_t)(value.at(2) >> 4)) * 100000) + (((int32_t)(value.at(2) & 0x0F)) * 10000) + (((int32_t)(value.at(1) >> 4)) * 1000) + (((int32_t)(value.at(1) & 0x0F)) * 100) + (((int32_t)(value.at(0) >> 4)) * 10) + (value.at(0) & 0x0F));
        }
        else if(type == 12)
        {
            return std::make_shared<Variable>((((int32_t)(value.at(3) >> 4)) * 10000000) + (((int32_t)(value.at(3) & 0x0F)) * 1000000) + (((int32_t)(value.at(2) >> 4)) * 100000) + (((int32_t)(value.at(2) & 0x0F)) * 10000) + (((int32_t)(value.at(1) >> 4)) * 1000) + (((int32_t)(value.at(1) & 0x0F)) * 100) + (((int32_t)(value.at(0) >> 4)) * 10) + (value.at(0) & 0x0F));
        }
        else if(type == 14)
        {
            return std::make_shared<Variable>((((int64_t)(value.at(5) >> 4)) * 100000000000) + (((int64_t)(value.at(5) & 0x0F)) * 10000000000) + (((int64_t)(value.at(4) >> 4)) * 1000000000) + (((int64_t)(value.at(4) & 0x0F)) * 100000000) + (((int64_t)(value.at(3) >> 4)) * 10000000) + (((int64_t)(value.at(3) & 0x0F)) * 1000000) + (((int64_t)(value.at(2) >> 4)) * 100000) + (((int64_t)(value.at(2) & 0x0F)) * 10000) + (((int64_t)(value.at(1) >> 4)) * 1000) + (((int64_t)(value.at(1) & 0x0F)) * 100) + (((int64_t)(value.at(0) >> 4)) * 10) + (value.at(0) & 0x0F));
        }
    }
    catch(const std::exception& ex)
    {
        GD::bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(const Exception& ex)
    {
        GD::bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__, ex.what());
    }
    catch(...)
    {
        GD::bl->out.printEx(__FILE__, __LINE__, __PRETTY_FUNCTION__);
    }
    return std::make_shared<Variable>();
}

}
#pragma once

#include <fstream>
#include <string>
#include <sstream>

#include "output.h"
#include "../integration/AngleStep.h"
#include "../integration/angular.h"
#include "./debugger.h"

class AnglesFileHandler : public Output
{

public:

    AnglesFileHandler() : Output(fileName){}

    long TryGetAnglesFromFile(AngleStep** anglesContainer)
    {
        // 1. read file
        bool fileCanBeRead = TryReadFile();

        if(!fileCanBeRead)
        {
            CDebugger::log("Could not read AngleSteps from file - file is absent or empty.\n");
            return false;
        }

        // 2. parse data
        long numberOfAnglesParsed = TryParseAngles(anglesContainer);
        if(numberOfAnglesParsed == 0)
        {
            CDebugger::warn("Could not read AngleSteps from file. Something went wrong!\n");
        }
        else
        {   
            // 3. return success 
            CDebugger::log("AngleSteps were successfully read from file. Number of angles: %d\n", numberOfAnglesParsed);
        }
        return numberOfAnglesParsed;        
    }

    bool TryWriteAnglesToFile(AngleStep** anglesContainer)
    {
        // check if file is available
        bool fileCanBeWritten = TryWriteFile();

        if(!fileCanBeWritten)
        {
            CDebugger::log("Could not write AngleSteps from file - file is unavailable.\n");
            return false;
        }

        // print data to file
        long index = 0;
        AngleStep* angleStep = anglesContainer[0];
        std::string angleStepString;
        while (angleStep != nullptr)
        {
            bool serializedSuccessfully = TrySerializeAngleStep(angleStep, angleStepString);

            if(!serializedSuccessfully)
            {
                CDebugger::error("Couldn't serialize AngleStep to file. AngleStep index was: %d\n", index);
                break;
            }

            prt(angleStepString.c_str());

            index++;
            angleStep = anglesContainer[index];
        }
    }

    bool TryWriteAngleToFile(AngleStep *angleStep)
    {
        bool serializedSuccessfully = TrySerializeAngleStep(angleStep, angleStepString);

        if(!serializedSuccessfully)
        {
            CDebugger::error("Couldn't serialize AngleStep to file. AngleStep was: %d\n",0); // put here angle description
        }

        prt(angleStepString.c_str());
    }


private:

    char* fileName = "Angles.dat";
    char delimiter = '|';

    /// @brief Tries to open a file and checks is it is empty
    /// @return false if file absent or empty
    bool TryReadFile()
    {
        std::ifstream file(fileName, std::ios::ate);

        // check if file exists
        if (!file.is_open()) 
        {
            return false; 
        }

        // check if file is empty
        std::streampos size = file.tellg(); 
        
        file.close();
        return size > 0;
    }

    /// @brief Tries to open the file for writing
    /// @return false if couldn't open the file
    bool TryWriteFile()
    {
        std::ofstream file(fileName);

        if (!file.is_open())
        {
            return false;
        }

        file.close();
        return true;
    }

    long TryParseAngles(AngleStep** anglesContainer)
    {
        std::ifstream file(fileName);
        std::string line;
        long index = 0;

        while(std::getline(file, line))
        {
            if(!TryDeserializeAngleStep(line, anglesContainer[index]))
            {
                CDebugger::error("Couldn't deserialize AngleStep from file %s, line %d.\n", fileName, index);
                
                break;
            }

            index++;
        }

        file.close();
        return index;
    }

    bool TryDeserializeAngleStep(const std::string& angleDataString, AngleStep* angleStepAddress)
    {
        std::stringstream ss(angleDataString);
        std::string token;
        double phi, theta, dphi, dtheta;

        try
        {
            std::getline(ss, token, delimiter);
            phi = std::stod(token);

            std::getline(ss, token, delimiter);
            theta = std::stod(token);

            std::getline(ss, token, delimiter);
            dphi = std::stod(token);

            std::getline(ss, token, delimiter);
            dtheta = std::stod(token);

            // Clear the stream content
            ss.str("");   
            ss.clear();

            angleStepAddress = new AngleStep(phi, theta, dphi, dtheta);
        }
        catch(...)
        {
            return false;
        }

        return true;
    }

    bool TrySerializeAngleStep(AngleStep* angleStepAddress, std::string& angleText)
    {
        if(angleStepAddress == nullptr)
        {
            CDebugger::error("Couldn't serialize AngleStep");
            return false;
        }

        std::stringstream ss;

        ss << angleStepAddress->phi << delimiter << angleStepAddress->theta << delimiter 
           << angleStepAddress->dphi << delimiter << angleStepAddress->dtheta << "\n";
        angleText = ss.str();

        return true;
    }

};
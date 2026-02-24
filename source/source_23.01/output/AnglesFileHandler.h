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
    
    /// @brief Creates and AnglesFielHandler with its own specifics
    /// @param mode file access mode. We use 'append+' whick allows
    /// append data to the end of the file and read from this file
    AnglesFileHandler(const char* mode = "w+") : Output("Angles.dat", mode){}

    long TryGetAnglesFromFile(AngleStep** anglesContainer)
    {
        // 1. read file
        bool fileCanBeRead = true; //TryReadFile();

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

    bool TryWriteAnglesToFile(std::vector<AngleStep> angleSteps)
    {
        // check if file is available
        bool fileCanBeWritten = true;// TryWriteFile();

        if(!fileCanBeWritten)
        {
            CDebugger::error("Could not write AngleSteps to file - file is unavailable.\n");
            return false;
        }

        // print data to file
        CDebugger::log("WRITING ANGLE STEPS. NUMBER OF ANGLE STEPS: %d", angleSteps.size());
        std::string angleStepString;
        auto angleStep = angleSteps.begin();
        while (angleStep != angleSteps.end())
        {
            bool serializedSuccessfully = TrySerializeAngleStep(&(*angleStep), angleStepString);

            if(!serializedSuccessfully)
            {
                CDebugger::error("Couldn't serialize AngleStep to file. AngleStep index was: %d\n", 
                                std::distance(angleSteps.begin(), angleStep));
                break;
            }
            
            //CDebugger::log("ANGLE STEP SERIALIZED SUCCESFULLY");

            //auto angleStepStringData = angleStepString.c_str();
            //CDebugger::log("WRITING ANGLE STEP: %s", angleStepStringData);

            auto success = prt("%s", angleStepString.c_str());

            /*
            if(success)
                CDebugger::log("SUCCESSFULLY PRINTED");
            else
            {
                CDebugger::error("UNSUCCESSFULLY PRINTED");
                return false;
            }
            */

            ++angleStep;
        }
        return true;
    }

    bool TryWriteAngleToFile(AngleStep *angleStep)
    {
        std::string angleStepString;
        bool serializedSuccessfully = TrySerializeAngleStep(angleStep, angleStepString);

        if(!serializedSuccessfully)
        {
            CDebugger::error("Couldn't serialize AngleStep to file. AngleStep was: %d\n",0); // put here angle description
        }
        else
            CDebugger::log("ANGLE STEP SERIALIZED SUCCESFULLY");

        prt("%s", angleStepString.c_str());
        CDebugger::log("ANGLE STEP WRITTEN TO FILE SUCCESFULLY");
    }


private:

    const char* fileName = "Angles.dat";
    char delimiter = '|';

    /// @brief Tries to open a file and checks is it is empty
    /// @return false if file absent or empty
    bool TryReadFile()
    {
        std::ifstream file(fileName, std::ios::ate);

        // check if file exists
        if (!file.is_open()) 
        {
            CDebugger::error("CAN'T READ FILE");
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
        // this method created file in launch_scripts which is wrong
        std::ofstream file(fileName);

        if (!file.is_open())
        {
            CDebugger::error("CAN'T WRITE TO FILE");
            return false;
        }
        
        CDebugger::log("FILE IS READY TO WRITE");

        file.close();
        CDebugger::log("FILE CHECK IS FINISHED. FILE CLOSED");
        return true;
    }

    long TryParseAngles(AngleStep** anglesContainer)
    {
        char fullFileName[255];
		sprintf(fullFileName, "%s/%s",App::output_dir, fileName);

        CDebugger::log("FILE PATH TO READ ANGLES: %s", fullFileName);
        
        std::ifstream file(fullFileName);
        std::string line;
        long index = 0;
        //char cline[20];
        //file.getline(cline, 20);
        //CDebugger::log("TEST: %s", cline);
        while(std::getline(file, line))
        {
            //CDebugger::log("READING LINE: %s", line.c_str());
            if(!TryDeserializeAngleStep(line, &anglesContainer[index]))
            {
                CDebugger::error("Couldn't deserialize AngleStep from file %s, line %d.\n", fileName, index);
                break;
            }

            index++;
        }

        file.close();
        return index;
    }

    bool TryDeserializeAngleStep(const std::string& angleDataString, AngleStep** angleStepAddress)
    {
        std::setlocale(LC_ALL, "C");
        std::stringstream ss(angleDataString);
        std::string token;
        double phi, theta, dphi, dtheta;

        try
        {
            std::getline(ss, token, delimiter);
            //CDebugger::log("FIRST NUMBER (PHI) IS: '%s'", token.c_str());
            phi = std::stod(token);

            std::getline(ss, token, delimiter);
            //CDebugger::log("SECOND NUMBER (THETA) IS: '%s'", token.c_str());
            theta = std::stod(token);

            std::getline(ss, token, delimiter);
            //CDebugger::log("THIRD NUMBER (DPHI) IS: '%s'", token.c_str());
            dphi = std::stod(token);

            std::getline(ss, token, delimiter);
            //CDebugger::log("FOURTH NUMBER (DTHETA) IS: '%s'", token.c_str());
            dtheta = std::stod(token);

            // Clear the stream content
            ss.str("");   
            ss.clear();

            //CDebugger::log("Deserialized angle step: phi=%f; theta=%f; dphi=%f; dtheta=%f",
            //               phi, theta, dphi, dtheta);
            *angleStepAddress = new AngleStep(phi, theta, dphi, dtheta);
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
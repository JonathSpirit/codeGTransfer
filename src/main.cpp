/////////////////////////////////////////////////////////////////////////////////
// Copyright 2021 Guillaume Guillet                                            //
//                                                                             //
// Licensed under the Apache License, Version 2.0 (the "License");             //
// you may not use this file except in compliance with the License.            //
// You may obtain a copy of the License at                                     //
//                                                                             //
//     http://www.apache.org/licenses/LICENSE-2.0                              //
//                                                                             //
// Unless required by applicable law or agreed to in writing, software         //
// distributed under the License is distributed on an "AS IS" BASIS,           //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.    //
// See the License for the specific language governing permissions and         //
// limitations under the License.                                              //
/////////////////////////////////////////////////////////////////////////////////

#include "main.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <thread>

#include "serial/serial.h"

#include "C_string.hpp"
#include "C_checksum.hpp"
#include "CMakeConfig.hpp"

#define MAX_NUMOFDATA 100

using namespace std;

void ShowAllPorts()
{
    std::vector<serial::PortInfo> devices = serial::list_ports();

    for (serial::PortInfo& device : devices)
    {
        std::cout << device.port << " - " << device.description << " - " << device.hardware_id << std::endl;
    }
}

void PrintHelp()
{
    std::cout << "codeGTransfer usage :" << std::endl << std::endl;

    std::cout << "Set the input file to be transfered" << std::endl;
    std::cout << "\tcodeGTransfer --in=<path>" << std::endl << std::endl;

    std::cout << "Set the port name" << std::endl;
    std::cout << "\tcodeGTransfer --port=<name>" << std::endl << std::endl;

    std::cout << "Print all the available ports (and do nothing else)" << std::endl;
    std::cout << "\tcodeGGcompiler --showPorts" << std::endl << std::endl;

    std::cout << "Print the version (and do nothing else)" << std::endl;
    std::cout << "\tcodeGGcompiler --version" << std::endl << std::endl;

    std::cout << "Print the help page (and do nothing else)" << std::endl;
    std::cout << "\tcodeGTransfer --help" << std::endl << std::endl;

    std::cout << "Ask the user how he want to compile his file (interactive compiling)" << std::endl;
    std::cout << "\tcodeGTransfer --ask" << std::endl << std::endl;
}
void PrintVersion()
{
    std::cout << "codeGTransfer created by Guillaume Guillet, version " << CGT_VERSION_MAJOR << "." << CGT_VERSION_MINOR << std::endl;
}

int main(int argc, char **argv)
{
    std::string portName;
    std::string fileInPath;

    std::vector<std::string> commands(argv, argv + argc);

    if (commands.size() <= 1)
    {
        PrintHelp();
        return -1;
    }

    for (unsigned int i=1; i<commands.size(); ++i)
    {
        //Commands
        if ( commands[i] == "--help")
        {
            PrintHelp();
            return 0;
        }
        if ( commands[i] == "--version")
        {
            PrintVersion();
            return 0;
        }
        if ( commands[i] == "--showPorts")
        {
            ShowAllPorts();
            return 0;
        }
        if ( commands[i] == "--ask")
        {
            std::cout << "Please insert the input path of the file"<< std::endl <<"> ";
            std::getline(std::cin, fileInPath);

            ShowAllPorts();
            std::cout << "Please insert the port name"<< std::endl <<"> ";
            std::getline(std::cin, fileInPath);
            continue;
        }

        //Commands with an argument
        std::vector<std::string> splitedCommand;
        codeg::Split(commands[i], splitedCommand, '=');

        if (splitedCommand.size() == 2)
        {
            if ( splitedCommand[0] == "--in")
            {
                fileInPath = splitedCommand[1];
                continue;
            }
            if ( splitedCommand[0] == "--port")
            {
                portName = splitedCommand[1];
                continue;
            }
        }

        //Unknown command
        std::cout << "Unknown command : \""<< commands[i] <<"\" !" << std::endl;
        return -1;
    }

    if ( fileInPath.empty() )
    {
        std::cout << "No input file !" << std::endl;
        return -1;
    }
    if ( portName.empty() )
    {
        std::cout << "Undefined port !" << std::endl;
        return -1;
    }

    std::cout << "Input file : \""<< fileInPath <<"\"" << std::endl;
    std::cout << "Port name : \""<< portName <<"\"" << std::endl;

    ///Opening file
    std::ifstream fileIn( fileInPath, std::ios::binary | std::ios::ate); //get the file size
    if ( !fileIn )
    {
        std::cout << "Can't read the file \""<< fileInPath <<"\"" << std::endl;
        return -1;
    }
    unsigned int fileSize = fileIn.tellg();

    fileIn.clear();
    fileIn.seekg(0); //Return to the beginning of the file

    ///Opening port
    serial::Serial port(portName, 9600, serial::Timeout(50, 4000, 0, 4000, 0),
                        serial::bytesize_t::eightbits,
                        serial::parity_t::parity_none,
                        serial::stopbits_t::stopbits_one,
                        serial::flowcontrol_t::flowcontrol_none);

    if( !port.isOpen() )
    {
        std::cout << "Can't open the port \""<< portName <<"\"" << std::endl;
        return -1;
    }

    std::cout << "Saying hello ... ";

    port.write("$H#");
    std::string result = port.read(20);

    std::cout << result << std::endl;
    if (result != "HELLO\n")
    {
        std::cout << "The board didn't respond or sent a bad response !" << std::endl;
        return -1;
    }

    uint8_t dataBuffer[MAX_NUMOFDATA];
    uint32_t startAddress = 0;

    std::cout << "Sending a total of " << fileSize << " byte(s) ..." << std::endl << std::endl;

    while ( fileIn.good() )
    {
        std::string dataToSend = "$W";
        std::string dataSended;
        uint8_t numOfData = 0;
        uint8_t checksum = 0;

        fileIn.read(reinterpret_cast<char*>(dataBuffer), MAX_NUMOFDATA);
        numOfData = fileIn.gcount();
        if (numOfData == 0)
        {
            continue;
        }

        checksum = codeg::CalculateChecksum(dataBuffer, numOfData); //Calculate checksum

        codeg::PushUint8InString(checksum, dataToSend); //Push checksum
        codeg::PushUint8InString(checksum, dataSended);
        codeg::PushUint24InString(startAddress, dataToSend); //Push start address

        for (unsigned int i=0; i<numOfData; ++i)
        {
            codeg::PushUint8InString(dataBuffer[i], dataToSend); //Push data
            codeg::PushUint8InString(dataBuffer[i], dataSended);
        }
        dataToSend += '#';

        ///Writing

        std::cout << "Sending " << static_cast<unsigned int>(numOfData) << " byte(s) of data at address " << startAddress << " ... ";

        port.write(dataToSend);

        std::string resultWrite = port.read(20);

        std::cout << resultWrite << std::endl;
        if (resultWrite != "WRITED\n")
        {
            std::cout << "The board didn't respond or sent a bad response !" << std::endl;
            return -1;
        }

        ///Reading

        std::cout << "Reading " << static_cast<unsigned int>(numOfData) << " byte(s) at address " << startAddress << " ... " << std::endl;

        dataToSend = "$R";
        codeg::PushUint24InString(startAddress, dataToSend); //Push start address
        codeg::PushUint24InString(numOfData, dataToSend); //Push num of data
        dataToSend += '#';

        port.write(dataToSend);

        std::string resultRead = port.read(500);

        std::cout << resultRead << std::endl;
        if ( resultRead.size() != static_cast<unsigned int>(10+numOfData*3) )
        {
            std::cout << "The board didn't respond or sent a bad response !" << std::endl;
            return -1;
        }
        resultRead.erase(0, 6);
        resultRead.pop_back();

        ///Compare
        if ( resultRead != dataSended )
        {
            std::cout << "The board didn't do a successfully write/read to the memory !" << std::endl;
            return -1;
        }

        startAddress += numOfData;
    }

    std::cout << "The board successfully write/read to the memory !" << std::endl;

    return 0;
}

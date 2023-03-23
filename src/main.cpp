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

#include "serial/serial.hpp"

#include "C_string.hpp"
#include "C_checksum.hpp"
#include "CMakeConfig.hpp"

#define MAX_NUMOFDATA 100
#define SECTOR_SIZE 4096

enum MEMORY_MODEL : uint8_t
{
    MEMM_EEPROM = 0,
    MEMM_FLASH  = 1
};

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

    std::cout << "Set the memory model (must be eeprom, flash or default) default to eeprom" << std::endl;
    std::cout << "\tcodeGTransfer --model=<name>" << std::endl << std::endl;

    std::cout << "Disable writing and flash erase" << std::endl;
    std::cout << "\tcodeGTransfer --verify" << std::endl << std::endl;

    std::cout << "Disable flash erase" << std::endl;
    std::cout << "\tcodeGTransfer --noErase" << std::endl << std::endl;

    std::cout << "Set the start address, default 0" << std::endl;
    std::cout << "\tcodeGTransfer --start=<number>" << std::endl << std::endl;

    std::cout << "Set the port name" << std::endl;
    std::cout << "\tcodeGTransfer --port=<name>" << std::endl << std::endl;

    std::cout << "Print all the available ports (and do nothing else)" << std::endl;
    std::cout << "\tcodeGTransfer --showPorts" << std::endl << std::endl;

    std::cout << "Print the version (and do nothing else)" << std::endl;
    std::cout << "\tcodeGTransfer --version" << std::endl << std::endl;

    std::cout << "Print the help page (and do nothing else)" << std::endl;
    std::cout << "\tcodeGTransfer --help" << std::endl << std::endl;

    std::cout << "Ask the user how he want to transmit his file (interactive)" << std::endl;
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
    uint8_t memoryModel = MEMM_EEPROM;
    uint32_t startAddress = 0;

    std::string transmitBuffer;
    std::string receiveBuffer;

    bool enableWrite = true;
    bool enableFlashErase = true;

    std::vector<std::string> commands(argv, argv + argc);

    if (commands.size() <= 1)
    {
        PrintHelp();
        return -1;
    }

    for (std::size_t i=1; i<commands.size(); ++i)
    {
        //Commands
        if (commands[i] == "--help")
        {
            PrintHelp();
            return 0;
        }
        if (commands[i] == "--version")
        {
            PrintVersion();
            return 0;
        }
        if (commands[i] == "--showPorts")
        {
            ShowAllPorts();
            return 0;
        }
        if (commands[i] == "--ask")
        {
            std::cout << "Please insert the input path of the file"<< std::endl <<"> ";
            std::getline(std::cin, fileInPath);

            ShowAllPorts();
            std::cout << "Please insert the port name"<< std::endl <<"> ";
            std::getline(std::cin, fileInPath);
            continue;
        }
        if (commands[i] == "--verify")
        {
            enableWrite = false;
            continue;
        }
        if (commands[i] == "--noErase")
        {
            enableFlashErase = false;
            continue;
        }

        //Commands with an argument
        std::vector<std::string> splitedCommand;
        Split(commands[i], splitedCommand, '=');

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
            if ( splitedCommand[0] == "--model")
            {
                if (splitedCommand[1] == "eeprom")
                {
                    memoryModel = MEMM_EEPROM;
                }
                else if (splitedCommand[1] == "flash")
                {
                    memoryModel = MEMM_FLASH;
                }
                else if (splitedCommand[1] == "default")
                {
                    memoryModel = MEMM_EEPROM;
                }
                else
                {
                    std::cout << "Unknown memory model : \""<< splitedCommand[1] <<"\" !" << std::endl;
                    return -1;
                }
                continue;
            }
            if ( splitedCommand[0] == "--start")
            {
                try
                {
                    startAddress = std::stoul(splitedCommand[1]);
                }
                catch (std::exception& e)
                {
                    std::cout << "Can't convert \""<< splitedCommand[1] << "\" as a number !" << std::endl;
                    std::cout << e.what() << std::endl;
                    return -1;
                }
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
    std::ifstream::pos_type fileSize = fileIn.tellg();

    fileIn.clear();

    if (startAddress >= fileSize)
    {
        std::cout << "Can't start at address "<< startAddress <<", the file size is only "<< fileSize <<" bytes !" << std::endl;
        return -1;
    }

    fileIn.seekg(startAddress); //Return to the start address
    std::cout << "Starting address : " << startAddress << std::endl;

    ///Opening port
    serial::Serial port(portName, 9600, serial::Timeout(50, 4000, 0, 4000, 0),
                        serial::ByteSizes::EIGHT_BITS,
                        serial::Parity::NONE,
                        serial::StopBits::ONE,
                        serial::FlowControls::NONE);

    if( !port.isOpen() )
    {
        std::cout << "Can't open the port \""<< portName <<"\"" << std::endl;
        return -1;
    }

    std::cout << std::endl;

    std::cout << "Saying hello ... ";

    port.write("$H#");
    receiveBuffer = port.read(20);

    std::cout << receiveBuffer << std::endl;
    if (receiveBuffer != "HELLO\n")
    {
        std::cout << "The board didn't respond or sent a bad response !" << std::endl;
        return -1;
    }

    std::cout << "Get board information ... ";

    port.write("$I#");
    receiveBuffer = port.read(100);

    std::cout << std::endl << receiveBuffer << std::endl;
    if (receiveBuffer.empty())
    {
        std::cout << "The board didn't respond !" << std::endl;
        return -1;
    }

    std::cout << "Set memory model ... ";

    transmitBuffer = "$Mx#";
    transmitBuffer[2] = memoryModel + '0';
    port.write(transmitBuffer);
    receiveBuffer = port.read(20);

    std::cout << receiveBuffer << std::endl;
    if (receiveBuffer.size() == 2)
    {
        if ((receiveBuffer[0]-'0') != memoryModel)
        {
            std::cout << "The board returned a bad memory model !" << std::endl;
            return -1;
        }
    }
    else
    {
        std::cout << "The board didn't respond or sent a bad response !" << std::endl;
        return -1;
    }

    std::cout << std::endl;

    if (enableWrite)
    {
        if (memoryModel == MEMM_FLASH)
        {
            if (enableFlashErase)
            {
                uint8_t startSector = 0;
                uint8_t countSector = (fileSize/SECTOR_SIZE) + 1;

                std::cout << "Erasing from sector "<< static_cast<int>(startSector) <<" to sector " << static_cast<int>(startSector+countSector) << " ..."  << std::endl;
                transmitBuffer = "$FES";
                PushUint8InString(startSector, transmitBuffer);
                PushUint8InString(countSector, transmitBuffer);
                transmitBuffer += '#';

                port.write(transmitBuffer);
                receiveBuffer = port.read(40);
                std::cout << receiveBuffer << std::endl;
                if (receiveBuffer.find("ERASED") == std::string::npos)
                {
                    std::cout << "The board didn't respond or sent a bad response !" << std::endl;
                    return -1;
                }
            }
            else
            {
                std::cout << "Flash erase skipped" << std::endl;
            }
        }

        std::cout << "Write and verify a total of " << fileSize << " byte(s) ..." << std::endl << std::endl;
    }
    else
    {
        std::cout << "Write skipped" << std::endl;
        std::cout << "Verify only a total of " << fileSize << " byte(s) ..." << std::endl << std::endl;
    }

    uint8_t dataBuffer[MAX_NUMOFDATA];

    while ( fileIn.good() )
    {
        std::cout << (startAddress*100)/fileSize << "% done ..." << std::endl;

        transmitBuffer = "$W";
        std::string dataReadCompare;
        uint8_t numOfData = 0;
        uint8_t checksum = 0;

        fileIn.read(reinterpret_cast<char*>(dataBuffer), MAX_NUMOFDATA);
        numOfData = fileIn.gcount();
        if (numOfData == 0)
        {
            continue;
        }

        checksum = CalculateChecksum(dataBuffer, numOfData); //Calculate checksum

        PushUint8InString(checksum, transmitBuffer); //Push checksum
        PushUint8InString(checksum, dataReadCompare);
        PushUint24InString(startAddress, transmitBuffer); //Push start address
        PushUint24InString(startAddress, dataReadCompare);

        for (unsigned int i=0; i<numOfData; ++i)
        {
            PushUint8InString(dataBuffer[i], transmitBuffer); //Push data
            PushUint8InString(dataBuffer[i], dataReadCompare);
        }
        transmitBuffer += '#';

        ///Writing
        if (enableWrite)
        {
            std::cout << "Writing " << static_cast<unsigned int>(numOfData) << " byte(s) of data at address " << startAddress << " ... ";

            port.write(transmitBuffer);

            receiveBuffer = port.read(20);

            std::cout << receiveBuffer << std::endl;
            if (receiveBuffer != "WRITED\n")
            {
                std::cout << "The board didn't respond or sent a bad response !" << std::endl;
                return -1;
            }
        }

        ///Reading

        std::cout << "Reading " << static_cast<unsigned int>(numOfData) << " byte(s) at address " << startAddress << " ... " << std::endl;

        transmitBuffer = "$R";
        PushUint24InString(startAddress, transmitBuffer); //Push start address
        PushUint24InString(numOfData, transmitBuffer); //Push num of data
        transmitBuffer += '#';

        port.write(transmitBuffer);

        receiveBuffer = port.read(500);

        std::cout << receiveBuffer << std::endl;
        if ( receiveBuffer.size() != static_cast<unsigned int>(18+numOfData*3) )
        {
            std::cout << "The board didn't respond or sent a bad response !" << std::endl;
            return -1;
        }
        receiveBuffer.erase(0, 6);
        receiveBuffer.pop_back();

        ///Compare
        if ( receiveBuffer != dataReadCompare )
        {
            std::cout << "The board didn't do a successfully write/read to the memory !" << std::endl;
            return -1;
        }

        startAddress += numOfData;
    }

    std::cout << "The board successfully write/read to the memory !" << std::endl;

    return 0;
}

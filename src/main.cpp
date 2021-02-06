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

#include "serial/serial.h"

#include "C_string.hpp"
#include "CMakeConfig.hpp"

using namespace std;

void ShowAllPorts()
{
    std::vector<serial::PortInfo> devices = serial::list_ports();

    for (serial::PortInfo& device : devices)
    {
        std::cout << device.port << " - " << device.description << " - " << device.hardware_id << std::endl;
    }
}

void printHelp()
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
void printVersion()
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
        printHelp();
        return -1;
    }

    for (unsigned int i=1; i<commands.size(); ++i)
    {
        //Commands
        if ( commands[i] == "--help")
        {
            printHelp();
            return 0;
        }
        if ( commands[i] == "--version")
        {
            printVersion();
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
    std::ifstream fileIn( fileInPath );
    if ( !fileIn )
    {
        std::cout << "Can't read the file \""<< fileInPath <<"\"" << std::endl;
        return -1;
    }

    ///Opening port
    serial::Serial port(portName, 9600, serial::Timeout::simpleTimeout(100),
                        serial::bytesize_t::eightbits,
                        serial::parity_t::parity_none,
                        serial::stopbits_t::stopbits_one,
                        serial::flowcontrol_t::flowcontrol_none);

    if( !port.isOpen() )
    {
        std::cout << "Can't open the port \""<< portName <<"\"" << std::endl;
        return -1;
    }

    port.write("$H#");
    std::string result = port.read(20);
    std::cout << result << std::endl;

    return 0;
}

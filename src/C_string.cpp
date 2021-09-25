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

#include "C_string.hpp"
#include <sstream>

size_t Split(const std::string& str, std::vector<std::string>& buff, char delimiter)
{
   std::string buffStr;
   std::istringstream strStream(str);
   while (std::getline(strStream, buffStr, delimiter))
   {
      buff.push_back(buffStr);
   }
   return buff.size();
}

void PushUint8InString(uint8_t value, std::string& str)
{
    char buff;
    buff = value/100 + '0';
    str += buff;
    buff = (value%100)/10 + '0';
    str += buff;
    buff = value%10 + '0';
    str += buff;
}

void PushUint24InString(uint32_t value, std::string& str)
{
    char buff;
    buff = value/10000000 + '0';
    str += buff;
    buff = (value%10000000)/1000000 + '0';
    str += buff;
    buff = (value%1000000)/100000 + '0';
    str += buff;
    buff = (value%100000)/10000 + '0';
    str += buff;
    buff = (value%10000)/1000 + '0';
    str += buff;
    buff = (value%1000)/100 + '0';
    str += buff;
    buff = (value%100)/10 + '0';
    str += buff;
    buff = (value%10) + '0';
    str += buff;
}

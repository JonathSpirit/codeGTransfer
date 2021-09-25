# codeG Transfer

Copyright (C) 2021 Guillaume Guillet

<table border="0px">
<tr>
<td>
Licensed under the Apache License, Version 2.0 (the "License");
</td>
</tr>
<tr>
<td>
You may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
</td>
</tr>
</table>

## Description
codeGTransfer is a utility for transmitting a compiled codeG file to the [memory module writer](https://github.com/JonathSpirit/MM1_Writer.git) board
and write the file into the memory.

codeG is a homemade binary language and codeGGenerator is the compiler [codeGGenerator](https://github.com/JonathSpirit/codeGGenerator)
[CodeG_binary](https://github.com/JonathSpirit/GComputer_standard).

you can find more info here : [MM1_Writer_code](https://github.com/JonathSpirit/MM1_Writer_code)

## Stats

![version](https://img.shields.io/badge/version-codeGTransfer_V0.2-blue)

![bug](https://img.shields.io/github/issues/JonathSpirit/codeGTransfer/bug)\
![fixed](https://img.shields.io/github/issues/JonathSpirit/codeGTransfer/fixed)

## Usage and exemple
```
codeGTransfer usage :

Set the input file to be transfered
        codeGTransfer --in=<path>

Set the memory model (must be eeprom, flash or default) default to eeprom
        codeGTransfer --model=<name>

Disable writing and flash erase
        codeGTransfer --verify

Disable flash erase
        codeGTransfer --noErase

Set the start address, default 0
        codeGTransfer --start=<number>

Set the port name
        codeGTransfer --port=<name>

Print all the available ports (and do nothing else)
        codeGTransfer --showPorts

Print the version (and do nothing else)
        codeGTransfer --version

Print the help page (and do nothing else)
        codeGTransfer --help

Ask the user how he want to compile his file (interactive compiling)
        codeGTransfer --ask
```

To transmit/write a file into the port COM3 with the flah memory model :\
``` codeGTransfer --in="input.cg" --port="COM3" --model="flash" ```

To verify the same file :\
``` codeGTransfer --in="input.cg" --port="COM3" --model="flash" --verify ```

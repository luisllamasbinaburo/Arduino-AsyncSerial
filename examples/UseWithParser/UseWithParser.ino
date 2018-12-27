/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 #include "AsyncSerialLib.h"
#include <ParserLib.h>

void parse(Parser& par, AsyncSerial& aSerial)
{
	par.Init(aSerial.GetContent(), aSerial.GetContentLength());
	Serial.println(par.Read_Uint16());
	par.SkipWhile(Parser::IsSeparator);
	Serial.println(par.Read_Uint16());
	par.Reset();
}

const int dataLength = 10;
byte data[dataLength];

Parser parser(data, dataLength);
AsyncSerial asyncSerial(data, dataLength, [](AsyncSerial& sender) { parse(parser, sender); });

// Open Serial port and write two numbers separated by . , ; - _ # = ?
void setup()
{
	while (!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");
}

void loop()
{
	asyncSerial.AsyncRecieve();
}
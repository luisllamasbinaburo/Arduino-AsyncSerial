/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
#include "AsyncSerialLib.h"

const int dataLength = 5;
byte data[dataLength] = { 65, 65, 65, 65, 65 };

void debug(String type)
{
	Serial.print(type);
	Serial.print("\t    at: ");
	Serial.print(millis());
	Serial.print(" ms ");
}

AsyncSerial asyncSerial(data, dataLength,
	[](AsyncSerial& sender) { Serial.println(); debug(String("Recieved")); Serial.println(); },
	[](AsyncSerial& sender) { Serial.println(); debug(String("Timeout")); Serial.println(); }
);

void setup()
{
	while (!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");

	asyncSerial.AutoReset = true;
	asyncSerial.Timeout = 5000;
	asyncSerial.AckChar = 'B';
}

// Open serial monitor. When recieve 'AAAAA', send 'B' as ACK, or do nothing to Timeout
void loop()
{
	asyncSerial.AsyncSend(true);
}
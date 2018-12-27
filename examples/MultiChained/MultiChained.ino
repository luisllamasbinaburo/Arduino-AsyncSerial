/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
#include "AsyncSerialLib.h"

const int dataLength1 = 5;
byte data1[dataLength1];

const int dataLength2 = 5;
byte data2[dataLength2];

void complete2(AsyncSerial& sender)
{
	Serial.print("  Token from AsyncSerial2: ");
	Serial.write(sender.GetContent(), sender.GetContentLength());
	Serial.println();
}
AsyncSerial asyncSerial2(data2, dataLength2, complete2);

void complete1(AsyncSerial& sender)
{
  asyncSerial2.ProcessByte(',');
  Serial.println("--- Line end from AsyncSerial1 ---");
}
AsyncSerial asyncSerial1(data1, dataLength2, complete1);

void setup()
{
	while (!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");

	asyncSerial1.OnByteProcessed = [](AsyncSerial& sender) {asyncSerial2.ProcessByte(sender.LastByte); };
	asyncSerial2.FinishChar = ',';
}


// Write 1,2,3 in Serial port monitor
void loop()
{
	asyncSerial1.AsyncRecieve();
}
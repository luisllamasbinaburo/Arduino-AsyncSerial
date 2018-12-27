/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
#ifndef _ASYNCSERIALLIB_h
#define _ASYNCSERIALLIB_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include "arduino.h"
#else
	#include "WProgram.h"
#endif

const char CARRIAGE_RETURN = '\r';
const char NEW_LINE = '\n';
const char ACK = '\0x06';

class AsyncSerial
{
	typedef void(*AsyncSerialCallback)(AsyncSerial& sender);

 public:
	 typedef enum
	 {
		 IDDLE,
		 RECIEVING_DATA,
		 RECIEVING_DATA_OVERFLOW,
		 MESSAGE_RECIEVED,
		 MESSAGE_RECIEVED_OVERFLOW,
		 TIMEOUT,
		 SENDING_DATA,
		 WAITING_ACK,
		 MESSAGE_SENDED
	}  Status;

	AsyncSerial(byte *buffer, size_t bufferLength, 
		AsyncSerialCallback OnRecievedOk, AsyncSerialCallback OnOverflow = nullptr, AsyncSerialCallback OnTimeout = nullptr );

	void Init(byte *buffer, size_t bufferLength, Stream* stream = NULL);

	Status AsyncRecieve();
	AsyncSerial::Status AsyncRecieve(int timeOut);
	AsyncSerial::Status Recieve(int timeOut);

	void AsyncSend(bool waitAck = false);
	void AsyncSend(byte* data, size_t dataLength, bool waitAck = false);
	void ProcessByte(byte data);
	void Send(bool waitAck = false);
	void Send(byte* data, size_t dataLength, bool waitAck = false);

	
	uint8_t GetLastIndex();
	uint8_t GetLastData();
	byte* GetContent();
	uint8_t GetContentLength();
	void OrderBuffer();
	void Start();
	void Stop();
	inline bool IsExpired();

	AsyncSerialCallback OnRecievedOk;
	AsyncSerialCallback OnOverflow;
	AsyncSerialCallback OnTimeout;
	AsyncSerialCallback OnByteProcessed;

	unsigned long Timeout = 0;
	bool AutoReset = true;
	bool AllowOverflow = false;
	bool SendAck;
	byte LastByte;
	char FinishChar = CARRIAGE_RETURN;
	char IgnoreChar = NEW_LINE;
	char AckChar = ACK;


 protected:
	 Stream* _stream;
	 byte *_buffer;
	 size_t _bufferIndex;
	 size_t _bufferLength;

	 unsigned long _startTime;
	 bool _sendAck = false;
	 Status _status;

	 void asyncRecieve();
	 void processNewData();
	 void finishRecieve();
	 static void orderBuffer(byte buffer[], size_t start, size_t end, size_t index);
	 static void swapBufferBlock(byte buffer[], size_t start, size_t length);
	 
	 void debugBuffer();
	 void debugStatus();
};

#endif


/***************************************************
Copyright (c) 2018 Luis Llamas
(www.luisllamas.es)

Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License
 ****************************************************/
 
#include "AsyncSerialLib.h"

AsyncSerial::AsyncSerial(byte* buffer, size_t bufferLength,
	AsyncSerialCallback onRecievedOk, AsyncSerialCallback onTimeout = nullptr, AsyncSerialCallback onOverflow = nullptr)
{
	Init(buffer, bufferLength, &Serial);
	OnRecievedOk = onRecievedOk;
	OnTimeout = onTimeout;
	OnOverflow = onOverflow;
	_startTime = millis();
}

void AsyncSerial::Init(byte *buffer, size_t bufferLength, Stream* stream)
{
	_status = RECIEVING_DATA;
	_startTime = millis();
	_stream = stream == NULL ? &Serial : stream;
	_buffer = buffer;
	_bufferLength = bufferLength;
	_bufferIndex = 0;
}


AsyncSerial::Status AsyncSerial::AsyncRecieve()
{
	if (_status == IDDLE) { return; }

	if (IsExpired())
	{
		if (OnTimeout != nullptr) OnTimeout(*this);
		_status = TIMEOUT;
	}

	if (_status >= MESSAGE_RECIEVED)
	{
		_startTime = millis();
		_status = AutoReset ? RECIEVING_DATA : IDDLE;
	}

	if (_status == RECIEVING_DATA || _status == RECIEVING_DATA_OVERFLOW)
	{
		asyncRecieve();
	}

	return _status;
}

AsyncSerial::Status AsyncSerial::AsyncRecieve(int timeOut)
{
	Timeout = timeOut;
	AsyncRecieve();
	return _status;
}

AsyncSerial::Status AsyncSerial::Recieve(int timeOut)
{
	_startTime = millis();
	_status = RECIEVING_DATA;

	bool expired = false;
	while (!expired && _status < MESSAGE_RECIEVED)
	{
		AsyncRecieve();
		expired = ((unsigned long)(millis() - _startTime) >= Timeout);
	}

	if (expired)
	{
		_status = TIMEOUT;
		if (OnTimeout != nullptr) OnTimeout(*this);
	}

	return _status;
}


void AsyncSerial::AsyncSend(bool waitAck)
{
	AsyncSend(_buffer, _bufferLength, waitAck);
}

void AsyncSerial::AsyncSend(byte* data, size_t dataLength, bool waitAck)
{
	if (_status == IDDLE) return;

	if (_status == TIMEOUT)
	{
		_status = AutoReset ? SENDING_DATA : IDDLE;
		return;
	}

	if (_status != WAITING_ACK)
	{
		_stream->write(data, dataLength);

		if (waitAck)
		{
			_status = WAITING_ACK;
			_startTime = millis();
		}
		else
		{
			_status = AutoReset ? MESSAGE_SENDED : IDDLE;
			if (OnRecievedOk != nullptr) OnRecievedOk(*this);
		}
	}

	if (_status == WAITING_ACK)
	{
		if (IsExpired())
		{
			_status = TIMEOUT;
			_startTime = millis();
			if (OnTimeout != nullptr) OnTimeout(*this);
		}
		else
		{
			if (_stream->read() == AckChar)
			{
				_status = AutoReset ? MESSAGE_SENDED : IDDLE;
				if (OnRecievedOk != nullptr) OnRecievedOk(*this);
			}
		}
	}
}

void AsyncSerial::ProcessByte(byte data)
{
	LastByte = data;

	if (data == (byte)FinishChar) finishRecieve();
	else processNewData();
}

void AsyncSerial::Send(bool waitAck)
{
	Send(_buffer, _bufferLength, waitAck);
}

void AsyncSerial::Send(byte* data, size_t dataLength, bool waitAck)
{
	_stream->write(data, dataLength);

	if (waitAck)
	{
		_startTime = millis();
		while (!IsExpired())
		{
			if (_stream->read() == AckChar)
			{
				_status = AutoReset ? MESSAGE_SENDED : IDDLE;
				if (OnRecievedOk != nullptr) OnRecievedOk(*this);
				return;
			}
		}
		_status = TIMEOUT;
		if (OnTimeout != nullptr) OnTimeout(*this);
	}
	else
	{
		_status = AutoReset ? MESSAGE_SENDED : IDDLE;
		if (OnRecievedOk != nullptr) OnRecievedOk(*this);
	}

}


uint8_t  AsyncSerial::GetLastIndex()
{
	return (_bufferIndex - 1 + _bufferLength) % _bufferLength;
}

byte AsyncSerial::GetLastData()
{
	return _buffer[GetLastIndex()];
}

byte * AsyncSerial::GetContent()
{
	return _buffer;
}

uint8_t AsyncSerial::GetContentLength()
{
	return _status == MESSAGE_RECIEVED_OVERFLOW ? _bufferLength : _bufferIndex;
}

void AsyncSerial::OrderBuffer()
{
	orderBuffer(_buffer, 0, _bufferLength - 1, GetLastIndex());
}

void AsyncSerial::Start()
{
	_status = RECIEVING_DATA;
	_bufferIndex = 0;
	_startTime = millis();
}

void AsyncSerial::Stop()
{
	_status = IDDLE;
}

inline bool AsyncSerial::IsExpired()
{
	if (Timeout == 0) return false;
	return ((unsigned long)(millis() - _startTime) > Timeout);
}


void AsyncSerial::asyncRecieve()
{
	while (_stream->available())
	{
		byte newData = _stream->read();

		ProcessByte(newData);
	}
}

void AsyncSerial::processNewData()
{
	if (LastByte != (byte)IgnoreChar)
	{
		if (OnByteProcessed != nullptr) OnByteProcessed(*this);

		if (_bufferIndex >= _bufferLength)
		{
			_bufferIndex %= _bufferLength;
			if (_status != RECIEVING_DATA_OVERFLOW)
			{
				if (OnOverflow != nullptr) OnOverflow(*this);
			}
			_status = RECIEVING_DATA_OVERFLOW;
		}

		_buffer[_bufferIndex] = LastByte;
		_bufferIndex++;
	}
}

void AsyncSerial::finishRecieve()
{
	_status = (_status == RECIEVING_DATA_OVERFLOW ? MESSAGE_RECIEVED_OVERFLOW : MESSAGE_RECIEVED);

	if (_status == MESSAGE_RECIEVED)
	{
		if (OnRecievedOk != nullptr) OnRecievedOk(*this);
	}
	else
	{
		if (AllowOverflow)
		{
			OrderBuffer();
			if (OnRecievedOk != nullptr) OnRecievedOk(*this);
			if (SendAck) _stream->write(AckChar);
		}
	}

	_bufferIndex = 0;
}

void AsyncSerial::orderBuffer(byte buffer[], size_t start, size_t end, size_t index)
{
	size_t leftBlockLength = index - start + 1;
	size_t rigthBlockLength = end - index;

	while (leftBlockLength != 0 && rigthBlockLength != 0)
	{
		if (leftBlockLength <= rigthBlockLength)
		{
			// RIGHT BLOCK SHIFT
			swapBufferBlock(buffer, start, leftBlockLength);
			start += leftBlockLength;
			index += leftBlockLength;
		}
		else
		{
			// LEFT BLOCK SHIFT
			swapBufferBlock(buffer, index - rigthBlockLength + 1, rigthBlockLength);
			end -= rigthBlockLength;
			index -= rigthBlockLength;
		}
		leftBlockLength = index - start + 1;
		rigthBlockLength = end - index;
	}
}

void AsyncSerial::swapBufferBlock(byte buffer[], size_t start, size_t length)
{
	byte temp;
	for (size_t i = 0; i < length; i++)
	{
		temp = buffer[start + i];
		buffer[start + i] = buffer[start + i + length];
		buffer[start + i + length] = temp;
	}
}


void AsyncSerial::debugStatus()
{
	switch (_status)
	{
		IDDLE: _stream->println("IDDLE"); break;
		RECIEVING_DATA: _stream->println("RECIEVING_DATA"); break;
		RECIEVING_DATA_OVERFLOW: _stream->println("RECIEVING_DATA_OVERFLOW"); break;
		MESSAGE_RECIEVED: _stream->println("MESSAGE_RECIEVED"); break;
		MESSAGE_RECIEVED_OVERFLOW: _stream->println("MESSAGE_RECIEVED_OVERFLOW"); break;
		TIMEOUT: _stream->println("TIMEOUT"); break;
		WAITING_ACK: _stream->println("WAITING_ACK"); break;
		MESSAGE_SENDED: _stream->println("	"); break;
		default: break;
	}
}

void AsyncSerial::debugBuffer()
{
	for (int i = 0; i < _bufferLength; i++)
	{
		_stream->print((char)_buffer[i]);
		_stream->print("\t");
	}
	_stream->println();

	for (int i = 0; i < _bufferLength; i++)
		_stream->print(i == GetLastIndex() ? "^" : "\t");
	_stream->println();
}

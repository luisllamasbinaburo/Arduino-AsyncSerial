# Librería Arduino AsyncSerial
La librería AsyncSerial implementa funciones para recibir y enviar un Stream de forma no bloqueante. En general, está diseñada para recibir datos por puerto serie sin interferir excesivamente el Loop principal.

La librería AsyncSerial trabaja con un buffer externo, que hay que proporcionar a la librería. Tras la recepción, los datos quedan almacenados en este buffer. La librería está diseñada para emplearse junto a la librería Parser, aunque cualquier otro medio de interpretación es posible.

Más información https://www.luisllamas.es/libreria-arduino-AsyncSerial/

## Instrucciones de uso
La librería AsyncSerial realiza la recepción o envío de un Stream de forma no bloqueante. Para ello emplea un buffer de bytes, que no es generado por la librería para evitar duplicar el espacio. En su lugar, este buffer debe ser facilitado a la librería en el constructor, aunque puede modificarse con la función 'Init(...).

Durante su funcionamiento, AsyncSerial modifica el contenido del buffer. Por tanto, no hay que almacenar ningún otro dato en el mismo, ni realizar su lectura mientras se esté emplando AsyncSerial. Por defecto, AsyncSerial emplea Serial para lectura y envío de datos, aunque puede ser modificado por cualquier otro Stream (otro Serial, SoftwareSerial, lectura de archivos, otra sistema de comunicación, etc...).

La función principal es AsyncRecieve(...), que recibe y almacena datos en el buffer hasta recibir un caracter delimitador. Por defecto, este caracter es '\r', ya que la mayoría de software envían este caracter como finalizador de línea junto a '\n', pero algunos software prescinden de '\n' (curiosamente) por lo que resulta más apropiado emplear '\r'. Por el mismo motivo, AsyncSerial ignora los caracteres '\n'. No obstante, puede cambiarse estos delimitadores cambiando los campos 'FinishChar e 'IgnoreChar.

La forma de realizar acciones de AsyncSerial es a través de funciones de callback, que reciben el AsyncSerial emisor como parámetro por referencia. Cuando AsyncSerial recibe el delimitador 'FinishChar se ejecuta la función de callback 'OnRecieveOk. Si el tiempo de Timeout expira, se ejecuta la función 'OnTimeout. Finalmente, si en algún momento se sobre pasa la longitud máxima buffer se ejecuta el método OnOverflow.

Con la variable 'AllowOverflow puede activarse o desactivarse el comportamiento ante desbordamiento del buffer. Por defecto es false, por lo que no el buffer se reinicia en caso de Overflow. En caso de activar 'AllowOverflow, se activa el comportamiento circular del buffer por lo que el buffer preserva los últimos elementos recibidos.

En caso de Overflow el buffer se comporta como un buffer circular. Es decir, el indice del buffer se desplaza, de forma que en cada momento se dispone de los últimos N elementos recibidos en el buffer. Cuando finaliza la recepción, AsyncSerial emplea un algoritmo de movimiento de bloques para ordenar de forma eficiente el buffer.

Por otro lado, se dispone de un Timeout. Si durante la recepción se excede el tiempo de Timeout se ejecuta la función de callback correspondiente, y se finaliza el proceso de recepción. Para desactivar el Timeout simplemente fijarlo a cero.

Tras una recepción válida, se debe emplear los métodos 'GetContent() y 'GetContentLength() para acceder a la información recibida por AsyncSerial. Es importante no exceder la longitud proporcionada por 'GetContentLength() dado que el resto del buffer contendrá información inválida de anteriores recepciones.

Pese a ser el método principal no bloqueantes, en algunas ocasiones resulta necesario esperar a que finalice la recepción, y no seguir ejecutando el código del programa hasta que esta finalice. Por ello se dispone del método 'Recieve, que dispone de una lógica similar a AsyncSerial pero bloquea la ejecución del programa durante la recepción.

También se dispone de los métodos 'AsyncSend(...) y 'Send(...), que realizan el envío de los datos del buffer. Ambos métodos permiten esperar un caracter de ACK, siendo la diferencia entre ambos si esta espera es bloqueante o no.

Adicionalmente, podemos elegir si queremos que el funcionamiento de AsyncSerial sea contínuo, o simple. En caso contínuo, AsyncSerial se reinicia al finalizar una recepción. En caso contrario, podemos controlarlo con las funciones 'Start() y 'Stop(). El modo de funcionamiento se controla con la variable 'AutoReset.

El estado de AsyncSerial está almacenado en la variable 'Status, que puede tomar los valores IDDLE, RECIEVING_DATA, RECIEVING_DATA_OVERFLOW, MESSAGE_RECIEVED, MESSAGE_RECIEVED_OVERFLOW, TIMEOUT, SENDING_DATA, WAITING_ACK, MESSAGE_SENDED. Dependiendo de la configuración (autoreset, waitACK) y método que usemos (recepción, envío), AsyncSerial pasará por unos u otros estados.

Por último, disponemos de la función de callback 'OnByteProcessed, que se dispara cada vez que AsyncSerial procesa un nuevo byte. Además disponemos de la función 'ProcessByte(...) que introduce manualmente un byte en AsyncSerial, sin tener que leer del Stream. Combinados, podemos combinar dos o más AsyncSerial para conseguir comportamientos avanzados.


### Constructor
La clase AsyncSerial se instancia a través de su constructor, que requiere un puntero al buffer externo, su longitud máxima, y la función 'OnRecieveOK. Opcionalmente podemos añadir las otras funciones de callback. 
```c++
AsyncSerial(byte *buffer, size_t bufferLength, AsyncSerialCallback OnRecievedOk, AsyncSerialCallback OnOverflow = nullptr, AsyncSerialCallback OnTimeout = nullptr );
```

No obstante, el origen y longitud del buffer, así como el Stream para las operaciones de lectura y escritura, pueden cambiarse con la función 'Init(...)
```c++
void Init(byte *buffer, size_t bufferLength, Stream* stream = NULL);
```

### Uso de AsyncSerial
El método principal de AsyncSerial es AsyncRecieve que, opcionalmente, admite un valor para el tiempo de Timeout (que también podemos fijar a través de una variable). Estas funciones devuelven el estado de AsyncSerial.
```c++
Status AsyncRecieve();
Status AsyncRecieve(int timeOut);
```

Por otro lado, podemos hacer una recepción bloqueante. En este caso es obligatorio especificar el tiempo de Timeout.
```c++
Status Recieve(int timeOut);
```

Cuando se finaliza la recepción, podemos obtener el contenido recibido a través de las funciones 'GetContent() y 'GetContentLength(). También es posible consultar el índice del último elemento respecto al buffer, y el último valor recibido.
```c++
byte* GetContent();
uint8_t GetContentLength();
uint8_t GetLastIndex();
uint8_t GetLastData();
```

Por otro lado, tenemos las funciones para enviar y recibir datos. Opcionalmente, se puede indicar si se desea esperar a que el recepto envíe un ACK. La diferencia entra 'AsyncSend(...) y 'Send(...) es si la espera del ACK es o no bloqueante.
```c++
void AsyncSend(bool waitAck = false);
void AsyncSend(byte* data, size_t dataLength, bool waitAck = false);
void Send(bool waitAck = false);
void Send(byte* data, size_t dataLength, bool waitAck = false);
```

Otras funciones y métodos que configurar el comportamiento de AsyncSerial son
```c++
void Start();				// Activa el AsyncSerial (si AutoReset == false)
void Stop();				// Pone el AsyncSerial en IDDLE (si AutoReset == false)
inline bool IsExpired();		// Devuelve True si AsyncSerial ha excedido el tiempo de Timeout
unsigned long Timeout = 0;		// Establece el tiempo de Timeout. Fijar a 0 para desactivar Timeout
bool AutoReset = true;			// Si es true, AsyncSerial se reiniciará tras recibir un mensaje válido
bool AllowOverflow = false;		// Controla el comportamiento ante desbordamiento
bool SendAck;				// Controla el envío y recepción de ACK
char FinishChar = CARRIAGE_RETURN;	// Caracter que finaliza el mensaje
char IgnoreChar = NEW_LINE;		// Caracter ignorado durante la recepción
char AckChar = ACK;			// Caracter de ACK enviado o recibido
```

## Ejemplos
La librería AsyncSerial incluye los siguientes ejemplos para ilustrar su uso.

* Simple: Muestra un ejemplo sencillo de uso de AsyncSerial.
```c++
#include "AsyncSerialLib.h"

const int dataLength = 10;
byte data[dataLength];

AsyncSerial asyncSerial(data, dataLength,
	[](AsyncSerial& sender) { Serial.write(sender.GetContent(), asyncSerial.GetContentLength());
				  Serial.println(); });

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
```

* AsyncRecieveContinuous: Muestra la recepción no bloqueante contínua.
```c++
#include "AsyncSerialLib.h"

const int dataLength = 5;
byte data[dataLength];

void debug(String type)
{
	Serial.print(type);
	Serial.print("\t    at: ");
	Serial.print(millis());
	Serial.print(" ms ");
}

void debugContent(AsyncSerial &serial)
{
	Serial.print("\t Length: ");
	Serial.print(serial.GetContentLength());
	Serial.print("   Content: ");
	Serial.write(serial.GetContent(), serial.GetContentLength());
}

AsyncSerial asyncSerial(data, dataLength,
	[](AsyncSerial& sender) { debug(String("Recieved")); debugContent(sender); Serial.println(); },
	[](AsyncSerial& sender) { debug(String("Timeout")); Serial.println(); },
	[](AsyncSerial& sender) { debug(String("Overflow")); Serial.println(); }
);


unsigned long start;
void setup()
{
	while (!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");

	asyncSerial.Timeout = 5000;
	asyncSerial.AllowOverflow = true;
}

void loop()
{
	asyncSerial.AsyncRecieve();
}

```

* AsyncRecieveSimple: Muestra la recepción no bloqueante sin AutoReset. Usa una función auxiliar para simular la activación y desactivación.
```c++
#include "AsyncSerialLib.h"

const int dataLength = 5;
byte data[dataLength];

void debug(String type)
{
	Serial.print(type);
	Serial.print("\t    at: ");
	Serial.print(millis());
	Serial.print(" ms ");
}

void debugContent(AsyncSerial &serial)
{
	Serial.print("\t Length: ");
	Serial.print(serial.GetContentLength());
	Serial.print("   Content: ");
	Serial.write(serial.GetContent(), serial.GetContentLength());
}

AsyncSerial asyncSerial(data, dataLength,
	[](AsyncSerial& sender) { debug(String("Recieved")); debugContent(sender); Serial.println(); },
	[](AsyncSerial& sender) { debug(String("Timeout")); Serial.println(); },
	[](AsyncSerial& sender) { debug(String("Overflow")); Serial.println(); }
);


unsigned long start;
void setup()
{
	while (!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");

	asyncSerial.Timeout = 5000;
	asyncSerial.AutoReset = false;
	asyncSerial.AllowOverflow = true;

	start = millis();
}

void loop()
{
	asyncSerial.AsyncRecieve();

	// Restart the AsyncSerial for example purposes
	if ((unsigned long)(millis() - start > 10000))
		Restart();

}

void Restart()
{
	Serial.print("-- Restarting example at ");
	while (Serial.available()) Serial.read(); //empty buffer
	Serial.print(millis());
	Serial.println("ms -- ");
	start = millis();
	asyncSerial.Start();
}
```

* Recieve: Muestra la recepción bloqueante de AsyncSerial.
```c++
#include "AsyncSerialLib.h"

const int dataLength = 5;
byte data[dataLength];

void debug(String type)
{
	Serial.print(type);
	Serial.print("\t    at: ");
	Serial.print(millis());
	Serial.print(" ms ");
}

void debugContent(AsyncSerial &serial)
{
	Serial.print("\t Length: ");
	Serial.print(serial.GetContentLength());
	Serial.print("   Content: ");
	Serial.write(serial.GetContent(), serial.GetContentLength());
}

AsyncSerial asyncSerial(data, dataLength,
	[](AsyncSerial& sender) { debug(String("Recieved")); debugContent(sender); Serial.println(); },
	[](AsyncSerial& sender) { debug(String("Timeout")); Serial.println(); },
	[](AsyncSerial& sender) { debug(String("Overflow")); Serial.println(); }
);


unsigned long start;
void setup()
{
	while (!Serial) { ; }
	Serial.begin(9600);
	Serial.println("Starting");

	asyncSerial.Timeout = 5000;
	asyncSerial.AutoReset = false;
	start = millis();
}

void loop()
{
	debug("Start recieving"); Serial.println();
	asyncSerial.Recieve(5000);
	debug("End recieving"); Serial.println();
}
```

* AsyncSend: Muestra el envío de datos con recepción no bloqueante del ACK.
```c++
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
```

* Multichain: Muestra el uso combinado de varios AsyncSerial, en este ejemplo para procesar un array separado por comas (no necesariamente la forma más eficiente, pero útil para un ejemplo).
```c++
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
```

* UserWithParser: Muestra el uso combinado con ArduinoParser para procesar los datos recibidos de forma no bloqueante.
```c++
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
```

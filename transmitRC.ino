/*
 * Para importar la libreria RC-Switch by sui77:
 * Arduino IDE > Programa > Incluir Libreria > Administrar Bibliotecas > Buscar: RC-Switch
 */
 
#define Xpin A5
#define Ypin A4
const int Bpin = 3;
const int transmitPin = 10;

#include <RCSwitch.h>

RCSwitch mySwitch = RCSwitch();
/* Colores en el autito: 
      0 - apagado
      1 - rojo
      2 - verde
      3 - azul
      4 - violeta
      5 - naranja
*/
int color = 0;
int cantColores = 6;
bool botonPresionado = false;

void setup() {
  pinMode(Xpin, INPUT);
  pinMode(Ypin, INPUT);
  pinMode(Bpin, INPUT_PULLUP);

  mySwitch.enableTransmit(transmitPin);      
  mySwitch.setRepeatTransmit(1);
  mySwitch.setProtocol(5);    //frecuencia = 666Hz
}

/*          CREAR_MASCARA
    todo 0 y pongo 1 entre el min y el max
          incluidos
*/
unsigned long crear_mascara (int max, int min)
{
  unsigned long mask = 0;
  int cantidad_unos = (max - min);
  int i;
  
  for (i=0; i<=(cantidad_unos-1); i++)
  {
    mask = mask + 1;
    mask = mask << 1;
  }
  mask = mask + 1;        //sino adentro del for queda 0 en el lsb
  
  mask = mask << min;
  
  return mask;    
}
//---------------------------------------------------------------------------------

// Funcion mySwitch.send(msg,32): el msg es el mensaje a transmitir y el 32 es el tamaño del mensaje
// Se envia un mensaje de 4 bytes
// Los 4 bytes se estructuran de la siguiente manera, siendo 1 el byte mas significativo, y 4 el menos significativo:
//  -> 1° byte: Valor analogico correspondiente a la posicion X del joystick.
//  -> 2° byte: Valor analogico correspondiente a la posicion Y del joystick.
//  -> 3° byte: Es un entero que representa el color que debera adoptar el auto.
//  -> 4° byte: Por el momento no esta en uso. Puede usarse para enviar el valor analogico de otro potenciometro para "Hackeos"

void enviarMensaje(int x, int y, int boton)   
{
  unsigned long mensaje = (unsigned long) x << 24;
  unsigned long posY = (unsigned long) y << 16;
  
  if (boton){ // Boton presionado
    if (!botonPresionado){ // Si no estaba presionado antes, aumento "color". Si ya estaba presionado, no hago nada
      color++;
      if (color >= cantColores) color = 0; // Si supera el maximo de colores se pone en 0
      botonPresionado = true;
    }
  }else{
    botonPresionado = false; // Si no estan apretando el boton marco que no esta presionado
  }

  mensaje = mensaje | (crear_mascara(23, 16) & posY) | (color << 8);
  mySwitch.send(mensaje, 32);
}
//-

void loop() {  
  delay(1);
  //se envian los estados del control mapeados entre 0 y 255
  enviarMensaje((int)map(analogRead(Xpin), 0, 1023, 255, 0), (int)map(analogRead(Ypin), 0, 1023, 255, 0), digitalRead(Bpin));
}

/*
 * Para importar la libreria RC-Switch by sui77:
 * Arduino IDE > Programa > Incluir Libreria > Administrar Bibliotecas > Buscar: RC-Switch
 */
 
#include <RCSwitch.h>

#define dirI_A 4
#define dirI_B 6
#define dirD_A 8
#define dirD_B 7
#define speedI 3
#define speedD 5
#define ledR 9
#define ledG 10
#define ledB 11

RCSwitch mySwitch = RCSwitch();
unsigned long ultimoRecibido = 0;
//unsigned int velocidadI = 0;
//unsigned int velocidadD = 0;
unsigned long actual = 0;
//unsigned long timer = 0;
//const unsigned long constTimer = 5000;
unsigned long valorRecibido = 0;
bool motoresEncendidos = false;
int matrizColores[6][3] = {{0,0,0},       //apagado
                           {255,0,0},     //rojo
                           {0,255,0},     //verde
                           {0,0,255},     //azul
                           {123,78,144},  //violeta
                           {254,149,91}}; //naranja    
                           //R...G...B                        
struct Lectura {
  int posX;
  int posY;
  int boton;
};

void setup() {
  mySwitch.enableReceive(0);  // Receiver on interrupt 0 => that is pin #2
  pinMode(speedI, OUTPUT);
  pinMode(speedD, OUTPUT);
  pinMode(dirI_A, OUTPUT);
  pinMode(dirI_B, OUTPUT);
  pinMode(dirD_A, OUTPUT);
  pinMode(dirD_B, OUTPUT);
  Serial.begin(9600);     //velocidad de transferencia 9600 (baud rate)
}

// crea mascara con 1s entre min y max incluidos
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
//-

//separar el value segun los bytes y mandar cada info al pin correspondiente
  Lectura getValores(unsigned long value)
  {
    Lectura l;
    
    unsigned long mascara = crear_mascara(31, 24);
    l.posX = (int)((value & mascara) >> 24);
    
    mascara = crear_mascara(23, 16);
    l.posY = (int)((value & mascara) >> 16);
    
    mascara = crear_mascara(15, 8);
    l.boton = (int) (value & mascara) >> 8;
    
    return l;
  } 
//-

//
  void setDir(char rueda, char dir)
  {
    int dirA = dirI_A;
    int dirB = dirI_B;

    if (rueda == 'D'){
      dirA = dirD_A;
      dirB = dirD_B;     
    }

    if (dir == 'F'){
      digitalWrite(dirA, LOW);
      digitalWrite(dirB, HIGH);
    }
    if (dir == 'B'){
      digitalWrite(dirA, HIGH);
      digitalWrite(dirB, LOW);
    }
    if (dir == 'S'){
      digitalWrite(dirA, LOW);
      digitalWrite(dirB, LOW);
    }    
  }
//-

//setear valores a los pines para dejar el auto quieto
  void resetear()
  {
    analogWrite(speedI, 0);
    analogWrite(speedD, 0);
    setDir('I', 'S');
    setDir('D', 'S');
  }
//-

//
  void setVelocidad(int x, int y, char dir)
  {
    unsigned int velocidad = sqrt((unsigned int)x*(unsigned int)x+(unsigned int)y*(unsigned int)y);
    //velocidad = velocidad*70/100; //para que no vaya tan rapido
    int proporcionI = 100;
    int proporcionD = 100;
    int p = 3;
    motoresEncendidos = true;
    
    switch (dir){

      case 'F': 
        setDir('I', 'F');
        setDir('D', 'F'); 

        if (y <= p*x && y >= -p*x){         //adelante a la derecha
          proporcionI = 100;
          proporcionD = 70;
        }else if (y >= p*x && y <= -p*x){   //adelante a la izquierda
          proporcionI = 70;
          proporcionD = 100;        
        }else if (y >= p*x && y >= -p*x){   //solo adelante
          proporcionI = 100;
          proporcionD = 100;
        }else{
          resetear();
        }

        break;

      case 'B':
        setDir('I', 'B');
        setDir('D', 'B'); 

        if (y <= p*x && y >= -p*x){       //atras a la derecha
          proporcionI = 100;
          proporcionD = 70;
        }else if (y >= p*x && y <= -p*x){ //atras a la izquierda
          proporcionI = 70;
          proporcionD = 100;        
        }else if (y <= p*x && y <= -p*x){ //solo atras 
          proporcionI = 100;
          proporcionD = 100;
        }else{
          resetear();
        }

        break;    

      case 'S':
        if (x > 10){
          setDir('I', 'F');             //derecha en el lugar
          setDir('D', 'B');
          proporcionI = 55;//proporcionI = 100;
          proporcionD = 55;//proporcionD = 70;
        }else if (x < -10){             //izquierda en el lugar
          setDir('I', 'B');
          setDir('D', 'F');
          proporcionI = 65;//proporcionI = 70;
          proporcionD = 65;//proporcionD = 100;
        }else{                          //quieto
          proporcionI = 0;
          proporcionD = 0;
        }

        break;

    }

    analogWrite(speedI, velocidad*proporcionI/100);
    //velocidadI = velocidad*proporcionI/100;
    analogWrite(speedD, velocidad*proporcionD/100); 
    //velocidadD = velocidad*proporcionD/100;
  }

//-

//x horizontal (costados) - y vertical (adelante, atras)
  void setMovimiento(int x, int y)
  {
    int posX = map(x, 0, 255, -255, 255);
    int posY = map(y, 0, 255, -255, 255);

    //Tolerancia del origen: entre -10 y 10
    if (posY > 10) setVelocidad(posX, posY, 'F');
    else if (posY < -10) setVelocidad(posX, posY, 'B');
    else setVelocidad(posX, posY, 'S');
  }
//-

//
  void setLeds(int boton)
  {
    analogWrite(ledR, matrizColores[boton][0]);
    analogWrite(ledG, matrizColores[boton][1]);
    analogWrite(ledB, matrizColores[boton][2]);    
  }
//-

void loop() {

  if (mySwitch.available()) {
    valorRecibido = mySwitch.getReceivedValue();
    ultimoRecibido = millis();
    
    Lectura l = getValores(valorRecibido);
    setMovimiento(l.posX, l.posY);
    setLeds(l.boton);
       
    mySwitch.resetAvailable();
    
  }else if (millis() - ultimoRecibido > 1000)resetear();     // si el ultimo valor fue recibido hace mucho tiempo, se resetea para evitar malas medidas. 

  /*if ((micros()-timer) >= constTimer){
    if (motoresEncendidos){
      motoresEncendidos = false;
      analogWrite(speedD, 0);
      analogWrite(speedI, 0);
    }
    else {
      motoresEncendidos = true;
      analogWrite(speedD, velocidadD);
      analogWrite(speedI, velocidadI);      
    }
    timer = micros();
  }*/
   
}

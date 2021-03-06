#include <LiquidCrystal.h>                        // Importa a biblioteca Para o display LCD
#include <SoftwareSerial.h>                       // Importa a biblioteca para utilizar os pinos 10, 11 como TX e RX
#include <Sim800l.h>                              // Importa a biblioteca para utilizar o módulo SIM800L
#include <IRremote.h>                             // Importa a biblioteca para utilizar o módulo IR
#include <NewTone.h>

LiquidCrystal display(2, 3, 4, 5, 6, 7);          // Instâcia do objeto display e os pinos que serão utilizados
Sim800l Sim800l;                                  // Instâcia do objeto Sim800l

boolean ligado = false;
boolean Sim800lEnable = false;

int Sensor = 13;                                  // Pino que será Utilizado como Sensor
int estadoSensor = 0;                             // Estado inicial do Sensor

int display_on = 8;

float seno;                                       // Define a variável seno que será utilizado para gerar o som da sirene
int frequencia;                                   // Define a variável frequencia que será utilizado para variar a frequencia do som da sirene

char* numero = "SEU_NUMERO_AQUI";                            // Número para o qual será enviada a notificação
char* NotificacaoSms;                             // Mensagem que será Enviada aos numeros cadastrados
boolean smsEnviado;
int pinSensorIR = 12;

int menuFuncoes = 1;


IRrecv sensorIR (pinSensorIR);
decode_results comando;


void setup() {

  Sim800l.begin();  // Inicia o objeto Sim800l
  NotificacaoSms = "UM MOVIMENTO FOI DETECTADO!";// Mensagem que será enviada

  Serial.begin(9600);                            // Inicia o monitor serial. Apenas para debug

  sensorIR.enableIRIn();                         // Inicia O sensorIR
  Serial.println("SensorIR Habilitado");

  display.begin(16, 2);                          // Inicia o display passando suas caracteristicas físicas: Número de colunas e linhas.

  pinMode(Sensor, INPUT);                        //Declara que Pino Sensor será utilizado como entrada
  pinMode(display_on, OUTPUT);
}


void loop() {
  if (sensorIR.decode(&comando)) {
    Serial.println(comando.value, HEX);
    switch (comando.value) {

      case (0xFF6897): {
          Serial.println("*");
          if (!ligado) {
            iniciarSistema();
            menuFuncoes = 1;
            funcoes();
          } else {
            desligarSistema();
          }
          break;
        }
      case (0xFFB04F): {
          Serial.println("#");
          if (ligado) {
            desligarSistema();
          }
          break;
        }
      case (0xFF18E7): {
          Serial.println("^");
          if (ligado) {
            menuFuncoes--;
            funcoes();
          }
          break;
        }
      case (0xFF4AB5): {
          Serial.println("+");
          if (ligado) {
            menuFuncoes++;
            funcoes();
          }
          break;
        }
      case (0xFF10EF): {
          Serial.println("<");
          break;
        }
      case (0xFF38C7): {
          Serial.println("OK");
          if (ligado) {
            if (menuFuncoes == 1) {
              calibrarSensor();
              monitoramento();
            }
            if (menuFuncoes == 2) {
              switchSMS();
              funcoes();
            }
            if (menuFuncoes == 3) {
              AlterarContatoSMS();
              funcoes();
            }
            if (menuFuncoes == 4) {
              desligarSistema();
            }
          }
          break;
        }
      case (0xFF5AA5): {
          Serial.println(">");
          break;
        }
    }
    sensorIR.resume();
  }
}




void alarme_on() {
  limpaDisplay();                             // Chama a função limpaDisplay, que irá restar os padroes do display

  if ((Sim800lEnable == true)) {
    Serial.println(numero);                   // Envia ao monitor Serial o número que deve receber a notificação
    Serial.println(NotificacaoSms);             // Envia ao monitor Serial a notificação

    display.setCursor(0, 0);                    // Move o cursor para a coluna 5 e linha 0 do display
    display.print("Alerta!");
    display.setCursor(8, 0);                    // Move o cursor para a coluna 5 e linha 1 do display
    display.print(numero);                    // Envia ao display o número que deve receber a notificação

    display.setCursor(5, 1);                    // Move o cursor para a coluna 0 e linha 1 do display
    display.print(NotificacaoSms);              // Envia ao display a notificação

    smsEnviado = Sim800l.sendSms(numero, NotificacaoSms);  // Envia SMS a número cadastrado com a notificação
    Serial.println("Envio do Sms retornou ");
    Serial.println(smsEnviado);
  }
  else {
    Serial.println(NotificacaoSms);             // Envia ao monitor Serial a notificação

    display.setCursor(5, 0);                    // Move o cursor para a coluna 5 e linha 0 do display
    display.print("Alerta!");

    display.setCursor(5, 1);                    // Move o cursor para a coluna 5 e linha 1 do display
    display.print(NotificacaoSms);              // Envia ao display a notificação


  }
  sirene();                                   // Chama a função sirene para gerar o som do alarme
}

void alarme_off() {
  limpaDisplay();
  display.print("Nenhum Movimento");
  display.setCursor(4, 1);
  display.print("Detectado ");
  noNewTone(9);
  delay(200);
}

void funcoes() {
  limpaDisplay();
  if (menuFuncoes < 1) {
    menuFuncoes = 4;
  }

  if (menuFuncoes > 4) {
    menuFuncoes = 1;
  }
  if (menuFuncoes == 1) {
    display.print("1 - Iniciar");
    display.setCursor(2, 1);
    display.print("Monitoramento");
  }

  if (menuFuncoes == 2) {
    if (Sim800lEnable) {
      display.print("2 - Desabilitar");
      display.setCursor(2, 1);
      display.print("Envio de SMS");
    } else {
      display.print("2 - Habilitar");
      display.setCursor(2, 1);
      display.print("Envio de SMS");
    }
  }

  if (menuFuncoes == 3) {
    display.print("3 - Alterar");
    display.setCursor(2, 1);
    display.print("Contato P/ SMS");

  }
  if (menuFuncoes == 4) {
    display.print("4 - Desligar");
  }
}


void iniciarSistema() {
  ligado = true;
  limpaDisplay();
  display.setCursor(3, 0);
  digitalWrite(display_on, HIGH);
  display.print("POWER_ON!");
  delay(3000);
}

void desligarSistema() {
  ligado = false;
  limpaDisplay();
  display.setCursor(3, 0);
  display.print("POWER_OFF!");
  delay(3000);
  display.clear();
  digitalWrite(display_on, LOW);
}

void monitoramento() {
  while (true) {

    estadoSensor = digitalRead(Sensor);         // A cada interação verifica o estado do pino Sensor

    if (sensorIR.decode(&comando)) {
      if ((comando.value) == (0xFFB04F)) {
        Serial.println("#");
        desligarSistema();
        return;
      }
      if ((comando.value) == (0xFF9867)) {
        Serial.println("0");
        funcoes();
        return;
      }

      sensorIR.resume();
    }

    if (estadoSensor == 1) {
      alarme_on();
      return;
    }

    if (estadoSensor == 0) {
      alarme_off();
    }
  }
}


void switchSMS() {
  limpaDisplay();
  Sim800lEnable = !Sim800lEnable;
  if (Sim800lEnable == true) {
    display.print("Envio de SMS");
    display.setCursor(5, 1);
    display.print("Habilitado!");
    delay(2000);
  }
  else {
    display.print("Envio de SMS");
    display.setCursor(3, 1);
    display.print("Desabilitado!");
    delay(2000);

  }
}



void AlterarContatoSMS() {
  sensorIR.resume();
  String novoNumero = "";

  limpaDisplay();
  display.print("Contato Atual:");
  display.setCursor(0, 1);
  display.print(numero);
  delay(2000);

  limpaDisplay();
  display.print("Novo Contato:");
  display.setCursor(0, 1);
  display.blink();

  while (true) {

    if (sensorIR.decode(&comando)) {
      Serial.println(comando.value, HEX);
      switch (comando.value) {

        case (0xFFA25D): {
            Serial.println("1");
            novoNumero = novoNumero + '1';
            display.print('1');
            break;
          }
        case (0xFF629D): {
            Serial.println("2");
            novoNumero = novoNumero + '2';
            display.print('2');
            break;
          }
        case (0xFFE21D): {
            Serial.println("3");
            novoNumero = novoNumero + '3';
            display.print('3');
            break;
          }
        case (0xFF22DD): {
            Serial.println("4");
            novoNumero = novoNumero + '4';
            display.print('4');
            break;
          }
        case (0xFF02FD): {
            Serial.println("5");
            novoNumero = novoNumero + '5';
            display.print('5');
            break;
          }
        case (0xFFC23D): {
            Serial.println("6");
            novoNumero = novoNumero + '6';
            display.print('6');
            break;
          }
        case (0xFFE01F): {
            Serial.println("7");
            novoNumero = novoNumero + '7';
            display.print('7');
            break;
          }
        case (0xFFA857): {
            Serial.println("8");
            novoNumero = novoNumero + '8';
            display.print('8');
            break;
          }
        case (0xFF906F): {
            Serial.println("9");
            novoNumero = novoNumero + '9';
            display.print('9');
            break;
          }
        case (0xFF9867): {
            Serial.println("0");
            novoNumero = novoNumero + '0';
            display.print('0');
            break;
          }
        case (0xFF38C7): {



            for (int x = 0; x < 11; x++) {
              numero[x] = char (novoNumero[x]);
            }

            Serial.println("OK");
            Serial.println(numero);
            limpaDisplay();
            display.noBlink();
            display.print("Sucesso!");
            display.setCursor(0, 1);
            display.print(numero);
            delay(2000);

            return;
            break;
          }
      }
      sensorIR.resume();
    }
  }
}

void limpaDisplay() {
  display.clear();
  display.home();
}

void calibrarSensor() {

  limpaDisplay();
  display.print("Calibrando");

  for (int c = 0; c < 6 ; c++) {
    //display.setCursor(13, 0);
    display.print(".");
    delay(1000);
  }
  display.setCursor(0, 1);
  display.print("Sensor Pronto!");
  delay(5000);

}

void sirene() {

  for (int x = 0; x < 15; x++) {
    display.scrollDisplayLeft();
    for (int x = 0; x < 180; x++) {
      //converte graus para radiando e depois obtém o valor do seno
      seno = (sin(x * 3.1416 / 180));
      //gera uma frequência a partir do valor do seno
      frequencia = 2000 + (int(seno * 100));
      NewTone(9, frequencia);
      delay(2);
    }
    if (sensorIR.decode(&comando)) {

      if ((comando.value) == (0xFF6897)) {
        Serial.println("*");
        noNewTone(9);
        desligarSistema();
        return;
      }
      if ((comando.value) == (0xFFB04F)) {
        Serial.println("#");
        noNewTone(9);
        desligarSistema();
        return;
      }
      if ((comando.value) == (0xFF9867)) {
        Serial.println("0");
        noNewTone(9);
        funcoes();
        return;
      }

      sensorIR.resume();
    }

  }
  if ((!smsEnviado) && (Sim800lEnable)) {
    noNewTone(9);
    smsEnviado = Sim800l.sendSms(numero, NotificacaoSms);  // Envia SMS a número cadastrado com a notificação
    Serial.println("Envio do Sms retornou ");
    Serial.println(smsEnviado);
  }
  sirene();
}

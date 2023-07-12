#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int encoderCLK = 2;
const int encoderDT = 3;
const int encoderSW = 4;

int lastEncoded;

int selectedVariable = 1;

int variable1 = 0;
int variable2 = 0;
int variable3 = 0;
bool ejercicioTerminado = false;

//servo 
Servo servoMotor;
int ang = 6;

//restricciones
const int variable1Min = 1;
const int variable1Max = 180;
const int variable2Min = 1;
const int variable2Max = 20;
const int variable3Min = 1;
const int variable3Max = 120;

void setup() {
  //inicializamos la pantalla
  lcd.init();
  lcd.backlight();

  //definimos pines encoder
  pinMode(encoderCLK, INPUT);
  pinMode(encoderDT, INPUT);
  pinMode(encoderSW, INPUT_PULLUP);
  lastEncoded = digitalRead(encoderCLK);

  //iniciamos el servo
  servoMotor.attach(ang);

  //MENSAJE DE BIENVENIDA
  lcd.setCursor(3, 0);
  String welcome = "Flexi-Codo";
  for (byte i = 0; i < welcome.length(); i++) {
    lcd.print(welcome[i]);
    delay(80);
  }
  delay(100);
  lcd.setCursor(5,1);
  lcd.print("V.1.2");
  delay(3000);

  //iniciando
  lcd.clear();
  lcd.print("Homing Servo");
  servoMotor.write(0);
  delay(1800);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1.Angulo:");
  lcd.setCursor(10, 1);
  lcd.print("grados");
}

void loop() {
  handleEncoderRotation();
  handleMenuSelection();
}

void handleEncoderRotation() {
  int state_clk = digitalRead(encoderCLK);
  int state_dt =digitalRead(encoderDT);

  if(lastEncoded == HIGH && state_clk == LOW){
    if(state_dt == LOW){
      decrementVariable();
    }else{
      incrementVariable();
    }
  }
  delay(5); //modificar si se clickea erroneamente
  lastEncoded = state_clk;
}
void incrementVariable() {
  switch (selectedVariable) {
    case 1:
      variable1++;
      break;
    case 2:
      variable2++;
      break;
    case 3:
      variable3++;
      break;
    default:
      break;
  }
  updateVariableDisplay();
}

void decrementVariable() {
  switch (selectedVariable) {
    case 1:
      variable1--;
      break;
    case 2:
      variable2--;
      break;
    case 3:
      variable3--;
      break;
    default:
      break;
  }
  updateVariableDisplay();
}

unsigned long lastButtonPressTime = 0;
const unsigned long buttonDebounceDelay = 200; // Intervalo de tiempo para evitar detección de pulsos múltiples (en milisegundos)

void handleMenuSelection() {
  if (digitalRead(encoderSW) == LOW && millis() - lastButtonPressTime >= buttonDebounceDelay) {
    lastButtonPressTime = millis(); // Registrar el tiempo del último pulso
  
    if (selectedVariable == 1) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("2.Pausa Ciclo:");
      lcd.setCursor(8, 1);
      lcd.print("segundos");
      selectedVariable++;
    } else if (selectedVariable == 2) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("3.Tiempo Total:");
      lcd.setCursor(9, 1);
      lcd.print("minutos");
      selectedVariable++;
    } else if (selectedVariable == 3) {
      lcd.clear();
      animaciondecarga();
      lcd.setCursor(0, 1);
      if(//rangos
        variable1 < variable1Min || variable1 > variable1Max ||
        variable2 < variable2Min || variable2 > variable2Max ||
        variable3 < variable3Min || variable3 > variable3Max ){
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Error:");
        lcd.setCursor(0, 1);
        lcd.print("Rango Invalido");
        delay(3000);
        ejercicioTerminado = true;
      }else{
      ejercicio();
      }
      if (ejercicioTerminado) {
        lcd.clear();
        selectedVariable = 1; // Vuelve al inicio
        lcd.setCursor(0, 0);
        lcd.print("1.Angulo:");
        lcd.setCursor(10, 1);
        lcd.print("grados");
      }
    }
  }
}

void updateVariableDisplay() {
  lcd.setCursor(0, 1);
  lcd.print("        "); // Limpiar la fila antes de imprimir el valor
  
  switch (selectedVariable) {
    case 1:
      lcd.setCursor(0, 1);
      lcd.print(variable1);
      break;
    case 2:
      lcd.setCursor(0, 1);
      lcd.print(variable2);
      break;
    case 3:
      lcd.setCursor(0, 1);
      lcd.print(variable3);
      break;
    default:
      break;
  }
}

void animaciondecarga(){
  // Animación de "Iniciando..." repetida 5 veces
  for (int i = 0; i < 6; i++) {
    lcd.print("Iniciando");
    delay(200);
    lcd.setCursor(9, 0);
    lcd.print(".");
    delay(200);
    lcd.setCursor(10, 0);
    lcd.print(".");
    delay(200);
    lcd.setCursor(11, 0);
    lcd.print(".");
    delay(200);
    lcd.setCursor(12, 0);
    lcd.print(".");
    delay(200);
    lcd.clear();
  }
}

void ejercicio(){

  int espera = 10; //fija
  //int angtrabajo=180;//variable 1 TEST
  int angtrabajo= variable1;
  //int tiempociclo = 1000; //variable 2 TEST
  int tiempociclo = variable2 * 1000; //variable 2 TEST
  //tiempototal= 15; //variable 3 TEST
  
  //calculan los ciclos
  int numciclos = (variable3 * 60000)/(2 * ((angtrabajo * espera) + tiempociclo)); //comentar para test
  //int numciclos=5; //descomentar para test corto
  lcd.setCursor(0, 0);
  lcd.print("Progreso:");
  lcd.setCursor(12, 0);
  lcd.print("|");
  lcd.print(numciclos);
  //ciclo de movimiento
  for (int ciclo = 0; ciclo < numciclos; ciclo++) {
    // 0 - ang
    for (int angle = 0; angle <= angtrabajo; angle++) {
      servoMotor.write(angle);
      delay(espera);
    }
  
    // tiempo de espera
    delay(tiempociclo);
  
    // ang - 0
    for (int angledec = angtrabajo; angledec >= 0; angledec--) {
      servoMotor.write(angledec);
      delay(espera);
    }
  
    // tiempo de espera
    delay(tiempociclo);
    updateProgressBar(ciclo, numciclos);
    lcd.setCursor(9, 0);
    lcd.print(ciclo+1);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  String mensaje1 = "Felicitaciones,";
  for (byte i = 0; i < mensaje1.length(); i++) {
    lcd.print(mensaje1[i]);
    delay(50);
  }
  lcd.setCursor(0, 1);
  String mensaje2 = "Nos vemos pronto!";
  for (byte i = 0; i < mensaje2.length(); i++) {
    lcd.print(mensaje2[i]);
    delay(50);
  }
  delay(5000);
  lcd.noBacklight();
  delay(1000);
  lcd.backlight();
  delay(1000);
  lcd.noBacklight();
  delay(1000);
  lcd.backlight();
  delay(1000);
  lcd.noBacklight();
  delay(1000);
  lcd.backlight();
  delay(1000);
  ejercicioTerminado = true;
  variable1= 0;
  variable2= 0;
  variable3= 0;
}

void updateProgressBar(int inicio, int total)
{
  int progress = map(inicio, 0, total, 0, 16);
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++)
  {
    if (i < progress)
    {
      lcd.print("#");
    }
    else
    {
      lcd.print(" ");
    }
  }
}

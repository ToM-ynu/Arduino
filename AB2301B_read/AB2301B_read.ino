
#include <Wire.h>
#include <CRC8.h>

#define AclSenAdrs 0x38
int AclSen;
byte get_data[7];

int count;

CRC8 crc;

byte initOpCodes[3] = {0xBE, 0x08, 0x00};
byte readFlags[3] = {0xAC, 0x33, 0x00};

struct packet
{
  byte state;
  int humidity;
  int temperature;
  byte res_CRC;
};
packet read_packet()
{
  packet pk;
  pk.state = get_data[0];
  pk.res_CRC = get_data[6];
  float hum_tmp, tem_tmp;
  hum_tmp = (get_data[1] << 12) + (get_data[2] << 4) + ((get_data[3] & 0xF0) >> 4);
  hum_tmp = hum_tmp / (1 << 20) * 100;

  tem_tmp = ((get_data[3] & 0x0F) << 16) + (get_data[4] << 8) + (get_data[5]);
  tem_tmp = tem_tmp / (1 << 20) * 200 - 50;
  pk.humidity = (int)hum_tmp;
  pk.temperature = (int)tem_tmp;
  return pk;
}

// the setup function runs once when you press reset or power the board
void setup()
{
  Wire.begin();
  Serial.begin(9600);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  while (1)
  {
    Serial.println("Check initial status expect 0x18");
    Wire.beginTransmission(AclSenAdrs);
    Wire.write(0x71);
    Wire.endTransmission();
    Wire.requestFrom(AclSenAdrs, 1);
    AclSen = Wire.read();
    Serial.println(AclSen, HEX);
    if (AclSen == 0x18)
      break;
    delay(1000);
    Serial.println("Force Initialization");
    Wire.beginTransmission(AclSenAdrs);
    Wire.write(initOpCodes, 3);
    Wire.endTransmission();
  }

}

// the loop function runs over and over again forever
void loop()
{
  crc.setPolynome(0x31);
    crc.setInitial(0xFF);
    Wire.beginTransmission(AclSenAdrs);
    Serial.println("Send Read flag");
    Wire.write(readFlags, 3);
    Wire.endTransmission();
    delay(160);
    Wire.beginTransmission(AclSenAdrs);
    Wire.write(0x71);
    Wire.endTransmission();

    Wire.requestFrom(AclSenAdrs, 7);
    for (int i = 0; i < 7; i++)
    {
      byte read_data = Wire.read();
      get_data[i] = read_data;
      if (i != 6)
        crc.add(read_data);
    }
    packet pk = read_packet();
    byte crc8_res = crc.calc();
    if (crc8_res == pk.res_CRC)
    {
      count++;
      Serial.print("humidity:\t");
      Serial.print(pk.humidity);
      Serial.print("\ttemperature:\t");
      Serial.println(pk.temperature);
    }
    else
    {
      Serial.println("CRC is not matched!");
    }
    crc.restart();
    digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
    delay(1000);                     // wait for a second
    digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
    delay(9000);

  // wait for a second
}

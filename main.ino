#include "HX711.h"				//тензо
#include <OneWire.h>			//температурный датчик
#include <DallasTemperature.h>	//температурный датчик
#include <Nextion.h>			//тач
//#include <Wire.h>

//актуатор
#define ENB 5
#define IN4 6
#define IN3 7
//помпа
#define IN2 8
#define IN1 9
#define ENA 10
//реле
#define RELAY 11
//термодатчик
#define TEMP 12

OneWire ds(TEMP);
DallasTemperature sensors(&ds);
DeviceAddress SensorRes = {0x28, 0xFF, 0xA1, 0x83, 0x54, 0x16, 0x04, 0x25}; //резервуар (с изолентой)
DeviceAddress SensorWater = {0x28, 0xFF, 0xE0, 0x84, 0x54, 0x16, 0x04, 0x87}; //бак с водой

float tempC; //значение температуры
int t = 0; //счетчик для обображения температуры и массы каждые n итераций
int weight; //масса
int page; //номер текущей страницы
char buffer[100] = {0}; //буффер для вывода текста
/*********************
*
*	page0 (главная)
*
**********************/
NexPage page0 = NexPage(0, 0, "page0");
NexButton b00 = NexButton(0, 2, "b0"); //монитор
NexButton b01 = NexButton(0, 3, "b1"); //температура
NexButton b02 = NexButton(0, 4, "b2"); //актуатор
NexButton b03 = NexButton(0, 5, "b3"); //весы
NexDSButton bt00 = NexDSButton(0, 6, "bt0"); //помпа
NexButton b04 = NexButton(0, 7, "b4"); //старт
/*********************
*
*	page1 (монитор)
*
**********************/
NexPage page1 = NexPage(1, 0, "page1");
NexButton b10 = NexButton(1, 2, "b0"); //back
/*********************
*
*	page2 (температура)
*
**********************/
NexPage page2 = NexPage(2, 0, "page2");
NexText t21 = NexText(2, 7, "t1"); //температура воды
NexText t22 = NexText(2, 8, "t2"); //температура резервуара
NexButton b20 = NexButton(2, 2, "b0"); //back
NexButton b21 = NexButton(2, 3, "b1"); //минус
NexButton b22 = NexButton(2, 4, "b2"); //плюс
NexDSButton bt20 = NexDSButton(2, 5, "bt0"); //включение/выключение реле
/*********************
*
*	page3 (актуатор)
*
**********************/
NexPage page3 = NexPage(3, 0, "page3");
NexButton b30 = NexButton(3, 2, "b0"); //back
NexButton b31 = NexButton(3, 3, "b1"); //минус
NexButton b32 = NexButton(3, 4, "b2"); //плюс
NexDSButton bt30 = NexDSButton(3, 6, "bt0"); //актуатор вниз
NexDSButton bt31 = NexDSButton(3, 7, "bt1"); //актуатор вверх
/*********************
*
*	page4 (весы)
*
**********************/
NexPage page4 = NexPage(4, 0, "page4");
NexText t41 = NexText(4, 6, "t1"); //вывод массы
NexButton b40 = NexButton(4, 2, "b0"); //back
NexButton b41 = NexButton(4, 3, "b1"); //оттаривание
NexButton b42 = NexButton(4, 4, "b2"); //смена ед. измерения
/*********************
*
*	page5 (старт)
*
**********************/
NexPage page5 = NexPage(5, 0, "page5");
NexButton b50 = NexButton(5, 2, "b0"); //back


NexTouch *nex_listen_list[] = 
{
	&page0,
	&b00,
    &b01,
    &b02,
    &b03,
    &bt00,
    &b04,

    &page1,
    &b10,

    &page2,
    &t21,
    &t22,
	&b20,
    &b21,
    &b22,
    &bt20,

    &page3,
    &b30,
    &b31,
    &b32,
    &bt30,
    &bt31,

    &page4,
    &b40,
    &b41,
    &b42,

    &page5,
    &b50,
    NULL
};

float readTemperature(const DeviceAddress TempAdr) //получение температуры с датчиков
{
	sensors.requestTemperaturesByAddress(TempAdr); // Отправить команду, чтобы получить температуру

	// получаем температуру
	tempC = sensors.getTempC(TempAdr);
	return tempC;
}

void b00PopCallback(void *ptr)
{
	page1.show();
	page = 1;
}

void b01PopCallback(void *ptr)
{
	page2.show();
	page = 2;
}
void b02PopCallback(void *ptr)
{
	page3.show();
	page = 3;
}
void b03PopCallback(void *ptr)
{
	page4.show();
	page = 4;
}
void bt00PopCallback(void *ptr)
{
	uint32_t dual_state;
	NexDSButton *btn = (NexDSButton *)ptr;
	bt00.getValue(&dual_state);
	if(dual_state)
	{
		//запуск помпы
		analogWrite(ENA, 255);
		//digitalWrite(13, HIGH);
	}
	else
	{
		analogWrite(ENA, 0);
		//digitalWrite(13, LOW);
	}
}
void b04PopCallback(void *ptr)
{
	page5.show();
	page = 5;
}
void b10PopCallback(void *ptr)
{
	page0.show();
	page = 0;
}
void b20PopCallback(void *ptr)
{
	page0.show();
	page = 0;
}
void b21PopCallback(void *ptr)
{
	
}
void b22PopCallback(void *ptr)
{
	
}
void bt20PopCallback(void *ptr)
{
	uint32_t dual_state;
	NexDSButton *btn = (NexDSButton *)ptr;
	bt20.getValue(&dual_state);
	if (dual_state)
	{
		digitalWrite(RELAY, 0);
	}
	else
	{
		digitalWrite(RELAY, 1);
	}
}
void b30PopCallback(void *ptr)
{
	page0.show();
	page = 0;
}
void b31PopCallback(void *ptr)
{
	
}
void b32PopCallback(void *ptr)
{
	
}
void bt30PopCallback(void *ptr)
{
	uint32_t dual_state;
	NexDSButton *btn = (NexDSButton *)ptr;
	bt30.getValue(&dual_state);
	if (dual_state)
	{
		digitalWrite(IN3, 1);
		digitalWrite(IN4, 0);
		analogWrite(ENB, 255);
	}
	else
	{
		digitalWrite(IN3, 0);
		digitalWrite(IN4, 1);
		analogWrite(ENB, 0);
	}
}
void bt31PopCallback(void *ptr)
{
	uint32_t dual_state;
	NexDSButton *btn = (NexDSButton *)ptr;
	bt31.getValue(&dual_state);
	if (dual_state)
	{
		digitalWrite(IN3, 0);
		digitalWrite(IN4, 1);
		analogWrite(ENB, 255);
	}
	else
	{
		digitalWrite(IN3, 0);
		digitalWrite(IN4, 1);
		analogWrite(ENB, 0);
	}
}
void b40PopCallback(void *ptr)
{
	page0.show();
	page = 0;
}
void b41PopCallback(void *ptr)
{
	
}
void b42PopCallback(void *ptr)
{
	
}
void b50PopCallback(void *ptr)
{
	page0.show();
	page = 0;
}

void setup()
{
	nexInit();
	b00.attachPop(b00PopCallback, &b00);
	b01.attachPop(b01PopCallback, &b01);
	b02.attachPop(b02PopCallback, &b02);
	b03.attachPop(b03PopCallback, &b03);
	bt00.attachPop(bt00PopCallback, &bt00);
	b04.attachPop(b04PopCallback, &b04);
	b10.attachPop(b10PopCallback, &b10);
	b20.attachPop(b20PopCallback, &b20);
	b21.attachPop(b21PopCallback, &b21);
	b22.attachPop(b22PopCallback, &b22);
	bt20.attachPop(bt20PopCallback, &bt20);
	b30.attachPop(b30PopCallback, &b30);
	b31.attachPop(b31PopCallback, &b31);
	b32.attachPop(b32PopCallback, &b32);
	bt30.attachPop(bt30PopCallback, &bt30);
	bt31.attachPop(bt31PopCallback, &bt31);
	b40.attachPop(b40PopCallback, &b40);
	b41.attachPop(b41PopCallback, &b41);
	b42.attachPop(b42PopCallback, &b42);
	b50.attachPop(b50PopCallback, &b50);

	//устанавливаем разрешение N бит - Допустимые значения 9, 10 или 11 бит.
	sensors.setResolution(SensorRes, 9);	//для датчика в резервуаре
	sensors.setResolution(SensorWater, 11);	//в воде

	Init_Hx711();
	Serial.begin(9600);
	Get_Maopi();
	//настройка помпы
	pinMode(IN1, OUTPUT);
	pinMode(IN2, OUTPUT);
	pinMode(ENA, OUTPUT);
	digitalWrite(IN1, 1);
	digitalWrite(IN2, 0);
	//analogWrite(ENA, 15);

	//настройка актуатора
	pinMode(IN3, OUTPUT);
	pinMode(IN4, OUTPUT);
	pinMode(ENB, OUTPUT);

	/*digitalWrite(IN3, 0);
	digitalWrite(IN4, 1);
	analogWrite(ENB, 255); */

	//реле
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, HIGH); //выключение реле (защита)
}

void loop()
{
	nexLoop(nex_listen_list);

	Serial.println(readTemperature(SensorWater));
	if (page == 0)
	{

	}
	if (page == 1)
	{

	}
	if (page == 2)
	{
		memset(buffer, 0, sizeof(buffer));
		itoa(readTemperature(SensorWater), buffer, 10);
		t21.setText(buffer);
		memset(buffer, 0, sizeof(buffer));
		itoa(readTemperature(SensorRes), buffer, 10);
		t22.setText(buffer);
	}
	if (page == 3)
	{

	}
	if (page == 4)
	{
		if (t == 10000)
		{
			weight = Get_Weight(); //получаем массу
			memset(buffer, 0, sizeof(buffer)); //очистка буффера
			itoa(weight, buffer, 10); //массу в буффер
			t41.setText(buffer);
			t = 0;
		}
		t++;
	}
	if (page == 5)
	{

	}
}
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
//прерывание для тача
//#define RX 2
//#define TX 3

OneWire ds(TEMP);
DallasTemperature sensors(&ds);
DeviceAddress SensorRes = {0x28, 0xFF, 0xA1, 0x83, 0x54, 0x16, 0x04, 0x25}; //резервуар (с изолентой)
DeviceAddress SensorWater = {0x28, 0xFF, 0xE0, 0x84, 0x54, 0x16, 0x04, 0x87}; //бак с водой

int tempRes = 20; //температура в резервуаре
int tempResCompare = 20;
int tempWater = 20; //температура воды
int tempWaterCompare = 20;
uint32_t pump_state_show = 0; //для единоразового показа состояния помпы (кнопки)
uint32_t heater_state_show = 0; //для единоразового показа состояния нагревателя (кнопки)
int weight = 0; //масса
int weightCompare = 0;
float weight_unit = 0.0;
uint8_t unit = 0;
uint8_t unitCompare = 0;
float unitCoeff = 1.0;
int page; //номер текущей страницы
char buffer[100] = {0}; //буффер для вывода текста
uint32_t pump_state = FALSE; //состояние помпы
uint32_t pump_stateCompare = FALSE;
uint32_t heater_state = FALSE; //состояние нагревателя
uint32_t heater_stateCompare = FALSE;
uint32_t heater_permission = FALSE; //разрешение на включение нагревателя
int act_speed = 16; //скорость актуатора
int act_speedCompare = 16;
int req_temp = 60; //требуемая температура в резервуаре
int req_tempCompare = 20;
uint8_t run = FALSE;
int timer = 20;
uint32_t started; //разница для millis()

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
NexText t10 = NexText(1, 3, "t0"); //температура резервуара
NexText t11 = NexText(1, 4, "t1"); //температура воды
NexText t12 = NexText(1, 5, "t2"); //требуемая температура
NexText t13 = NexText(1, 6, "t3"); //состояние нагревателя
NexText t14 = NexText(1, 7, "t4"); //скорость актуатора
NexText t15 = NexText(1, 8, "t5"); //значение массы
NexText t16 = NexText(1, 9, "t6"); //состояние помпы
/*********************
*
*	page2 (температура)
*
**********************/
NexPage page2 = NexPage(2, 0, "page2");
NexText t20 = NexText(2, 6, "t0"); //требуемая температура
NexText t21 = NexText(2, 7, "t1"); //температура воды
NexText t22 = NexText(2, 8, "t2"); //температура резервуара
NexText t23 = NexText(2, 9, "t3"); //предупреждение о включении насоса
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
NexText t30 = NexText(3, 5, "t0"); //скорость актуатора
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
NexText t40 = NexText(4, 5, "t0"); //единица измерения
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
NexDSButton bt50 = NexDSButton(5, 4, "bt0"); //старт/стоп
NexDSButton bt51 = NexDSButton(5, 6, "bt1"); //ось Х (сек/мм)
NexText t50 = NexText(5, 5, "t0"); //таймер
NexWaveform s50 = NexWaveform(5, 3, "s0"); //график

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
	&bt50,
	&bt51,
    NULL
};

int readTemperature(DeviceAddress TempAdr) //получение температуры с датчиков
{
	int tempC; //значение температуры
	sensors.requestTemperaturesByAddress(TempAdr); // Отправить команду, чтобы получить температуру

	// получаем температуру
	tempC = sensors.getTempC(TempAdr);
	return tempC;
}

void b00PopCallback(void *ptr)
{
	page1.show();
	page = 1;
	tempResCompare = 0;
	tempWaterCompare = 0;
	act_speedCompare = 0;
	weightCompare = 0;
	req_tempCompare = 0;
	pump_stateCompare = 0;
	heater_stateCompare = 0;
}

void b01PopCallback(void *ptr)
{
	page2.show();
	page = 2;
	heater_state_show = 1;
	req_tempCompare = 0; //костыль, чтобы показать требуемую температуру
	tempResCompare = 0;
}
void b02PopCallback(void *ptr)
{
	page3.show();
	page = 3;
	act_speedCompare = 0; //костыль, чтобы показать скорость актуатора
}
void b03PopCallback(void *ptr)
{
	page4.show();
	page = 4;
	weightCompare = 0;
	unit = 0;
	unitCoeff = 1;
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
		pump_state = TRUE;
		pump_state_show = 1;
	}
	else
	{
		analogWrite(ENA, 0);
		pump_state = FALSE;
		pump_state_show = 1;
		heater_permission = FALSE;
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
	pump_state_show = 1;
}
void b20PopCallback(void *ptr)
{
	page0.show();
	page = 0;
	pump_state_show = 1;
}
void b21PopCallback(void *ptr)
{

	if (req_temp > 20)
		req_temp -= 5;
}
void b22PopCallback(void *ptr)
{
	if (req_temp < 95)
		req_temp += 5;
}
void bt20PopCallback(void *ptr)
{
	uint32_t dual_state;
	NexDSButton *btn = (NexDSButton *)ptr;
	bt20.getValue(&dual_state);
	if (dual_state)
	{
		if (!pump_state)
			t23.setText("Please activate the pump");
		else 
		{
			heater_permission = TRUE;
			heater_state_show = 1;
		}
	}
	else
	{
		heater_permission = FALSE;
		heater_state_show = 1;
	}
}
void b30PopCallback(void *ptr)
{
	page0.show();
	page = 0;
	pump_state_show = 1;
}
void b31PopCallback(void *ptr)
{
	if (act_speed > 11)
		act_speed--;
}
void b32PopCallback(void *ptr)
{
	if (act_speed < 16)
		act_speed++;
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
	pump_state_show = 1;
}
void b41PopCallback(void *ptr)
{
	Get_Maopi();
}
void b42PopCallback(void *ptr)
{
	if (unit == 2)
		unit = 0;
	else 
		unit++;
	switch (unit)
	{
		case 0:
			unitCoeff = 1;
			t40.setText("g");
			break;
		case 1:
			unitCoeff = 0.03527;
			t40.setText("oz"); //унция
			break;
		case 2:
			unitCoeff = 0.0022;
			t40.setText("lb"); //фунт
			break;
		default:
		break;
	}
}
void b50PopCallback(void *ptr)
{
	if (!run)
	{
		page0.show();
		page = 0;
		pump_state_show = 1;
	}
}
void bt50PopCallback(void *ptr)
{
	uint32_t dual_state;
	NexDSButton *btn = (NexDSButton *)ptr;
	bt50.getValue(&dual_state);
	if (dual_state)
	{
		if (!run)
		{
			run = TRUE;
			timer = 20;
			started = millis();
		}
	}
	else
	{
		run = FALSE;
	}
}
void bt51PopCallback(void *ptr)
{

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
	bt50.attachPop(bt50PopCallback, &bt50);
	bt51.attachPop(bt51PopCallback, &bt51);

	//устанавливаем разрешение N бит - Допустимые значения 9, 10 или 11 бит.
	sensors.setResolution(SensorRes, 9);	//для датчика в резервуаре
	sensors.setResolution(SensorWater, 11);	//в воде

	Init_Hx711();
	Serial.begin(9600);
	delay(500);
	Get_Maopi(); //оттаривание
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

	//реле
	pinMode(RELAY, OUTPUT);
	digitalWrite(RELAY, HIGH); //выключение реле (защита)
}

void loop()
{
	nexLoop(nex_listen_list);
	//Serial.println(readTemperature(SensorWater));

	//заранее читаем датчики
	tempRes = readTemperature(SensorRes);
	tempWater = readTemperature(SensorWater);
	weight = Get_Weight(); //получаем массу
	
	//проверяем температуру резервуара (включаем/выключаем реле)
	if ((tempRes < req_temp) && heater_permission)
	{
		if (!heater_state)
		{
			heater_state = TRUE;
			digitalWrite(RELAY, 0);
		}
	}
	else 
	{
		if (heater_state)
		{
			heater_state = FALSE;
			digitalWrite(RELAY, 1);
		}
	}

	switch(page)
	{
		case 0:
			if (pump_state && pump_state_show)
			{
				pump_state_show = 0;
				bt00.setValue(pump_state);
			}
			break;
		case 1:
			if (tempRes != tempResCompare)
			{
				tempResCompare = tempRes;
				memset(buffer, 0, sizeof(buffer));
				itoa(tempResCompare, buffer, 10);
				t10.setText(buffer);
			}
			if (tempWater != tempWaterCompare)
			{
				tempWaterCompare = tempWater;
				memset(buffer, 0, sizeof(buffer));
				itoa(tempWaterCompare, buffer, 10);
				t11.setText(buffer);
			}
			if (req_temp != req_tempCompare)
			{
				req_tempCompare = req_temp;
				memset(buffer, 0, sizeof(buffer));
				itoa(req_tempCompare, buffer, 10);
				t12.setText(buffer);
			}
			if (weight != weightCompare)
			{
				weightCompare = weight;
				memset(buffer, 0, sizeof(buffer));
				itoa(weightCompare, buffer, 10);
				t15.setText(buffer);
			}
			if (act_speed != act_speedCompare)
			{
				act_speedCompare = act_speed;
				memset(buffer, 0, sizeof(buffer));
				itoa(act_speedCompare, buffer, 10);
				t14.setText(buffer);
			}
			if (pump_state != pump_stateCompare)
			{
				pump_stateCompare = pump_state;
				t16.setText("ON");				
			}
			if (heater_state != heater_stateCompare)
			{
				heater_stateCompare = heater_state;
				if (heater_stateCompare)
					t13.setText("ON");
				else 
					t13.setText("OFF");
			}
			break;
		case 2:
			if (heater_permission && heater_state_show == 1)
			{
				heater_state_show = 0;
				bt20.setValue(heater_permission);
			}
			if (tempRes != tempResCompare)
			{
				tempResCompare = tempRes;
				memset(buffer, 0, sizeof(buffer));
				itoa(tempResCompare, buffer, 10);
				t22.setText(buffer);
			}
			if (tempWater != tempWaterCompare)
			{
				tempWaterCompare = tempWater;
				memset(buffer, 0, sizeof(buffer));
				itoa(tempWaterCompare, buffer, 10);
				t21.setText(buffer);
			}
			if (req_temp != req_tempCompare)
			{
				req_tempCompare = req_temp;
				memset(buffer, 0, sizeof(buffer));
				itoa(req_tempCompare, buffer, 10);
				t20.setText(buffer);
			}
			break;
		case 3:
			if (act_speed != act_speedCompare)
			{
				act_speedCompare = act_speed;
				memset(buffer, 0, sizeof(buffer));
				itoa(act_speedCompare, buffer, 10);
				t30.setText(buffer);
			}
			break;
		case 4:
			if ((weight != weightCompare) || (unit != unitCompare))
			{
				unitCompare = unit;
				weightCompare = weight;
				if (unit != 0)
				{
					weight_unit = weightCompare * unitCoeff;
					memset(buffer, 0, sizeof(buffer)); //очистка буффера
					//itoa(weightCompare, buffer, 10); //массу в буффер
					dtostrf(weight_unit, 3, 3, buffer);
					t41.setText(buffer);
				}
				else 
				{
					memset(buffer, 0, sizeof(buffer)); //очистка буффера
					itoa(weightCompare, buffer, 10); //массу в буффер
					//dtostrf(weight_unit, 3, 3, buffer);
					t41.setText(buffer);
				}
			}
			break;
		case 5:
			if (run)
			{
				//static uint32_t started = millis();
				if ((millis() - started >= 1000) && timer > 0)
				{
					timer = timer - (millis() - started)/1000; 
					started = millis();
					memset(buffer, 0, sizeof(buffer));
					itoa(timer, buffer, 10);
					t50.setText(buffer);
					//опускаем актуатор
					digitalWrite(IN3, 1);
					digitalWrite(IN4, 0);
					analogWrite(ENB, 255);

					if (timer < 2)
					{
						started = millis();
						Get_Maopi();
					}
				}
				//else if(millis() - started >= 2000) started = millis();

				if (timer == 0)
				{
					//поднимаем актуатор
					digitalWrite(IN3, 1);
					digitalWrite(IN4, 0);
					//analogWrite(ENB, map(act_speed, 11, 16, 128, 255));
					if (((millis() - started) / 1000) % 2 == 0)
						s50.addValue(0, weight); //рисуем график каждые 2 секунды
					Serial.print(millis() - started);
					Serial.print(":");
					Serial.print(weight);
					Serial.println(";");
					if ((millis() - started) > 40000)
						run = FALSE;
				}
			}
			else 
				analogWrite(ENB, 0); //останавливаем актуатор
			break;
		default:
			//
		break;
	}
}
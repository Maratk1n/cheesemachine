#include "HX711.h"

long HX711_Buffer = 0;
long Weight_Maopi = 0, Weight_Shiwu = 0;

//****************************************************
//Инициализация HX711
//****************************************************
void Init_Hx711()
{
	pinMode(HX711_SCK, OUTPUT);	
	pinMode(HX711_DT, INPUT);
}


//****************************************************
//Получение пушной массы
//****************************************************
void Get_Maopi()
{
	HX711_Buffer = HX711_Read();
	Weight_Maopi = HX711_Buffer/100;		
} 

//****************************************************
//Взвешивание
//****************************************************
unsigned int Get_Weight()
{
	HX711_Buffer = HX711_Read();
	HX711_Buffer = HX711_Buffer/100;

	Weight_Shiwu = HX711_Buffer;
	Weight_Shiwu = Weight_Shiwu - Weight_Maopi;				//Получить значение выборки AD в натуральной форме.
	
	Weight_Shiwu = - (unsigned int)((float)Weight_Shiwu/9.45); 	
	// Вычислить реальный физический вес
	// Поскольку разные датчика характеристической кривой не то же самое, так что каждый датчик и исправить делитель 4.30 здесь.
	// Когда они узнают вес теста является слишком большим, увеличьте значение.
	// Если тест из массы слишком мал, уменьшить изменение значения.
	// Это значение обычно составляет около 7,16. Поскольку различные датчики могут быть.
	//+0.05 Процент округления
	return Weight_Shiwu;
}

//****************************************************
//Read HX711
//****************************************************
unsigned long HX711_Read(void)	//Прирост 128
{
	unsigned long count; 
	unsigned char i;
	bool Flag = 0;

	digitalWrite(HX711_DT, HIGH);
	delayMicroseconds(1);

	digitalWrite(HX711_SCK, LOW);
	delayMicroseconds(1);

  	count=0; 
  	while(digitalRead(HX711_DT)); 
  	for(i=0;i<24;i++)
	{ 
	  	digitalWrite(HX711_SCK, HIGH); 
		delayMicroseconds(1);
	  	count=count<<1; 
		digitalWrite(HX711_SCK, LOW); 
		delayMicroseconds(1);
	  	if(digitalRead(HX711_DT))
			count++; 
	} 
 	digitalWrite(HX711_SCK, HIGH); 
	count ^= 0x800000;
	delayMicroseconds(1);
	digitalWrite(HX711_SCK, LOW); 
	delayMicroseconds(1);
	
	return(count);
}

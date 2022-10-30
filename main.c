#include <reg52.h>

// define control singtal for LED and BUZZER
#define TURN_ON_LED 1
#define TURN_OFF_LED 2
#define TURN_ON_BUZZER 3
#define TURN_OFF_BUZZER 4

unsigned char RECEIVED_SIGNAL = 0;

void ConfigUART(unsigned int baud);
void SignalDriver();


// define LED related members
sbit ADDR_0 = P1^0;
sbit ADDR_1 = P1^1;
sbit ADDR_2 = P1^2;
sbit ADDR_3 = P1^3;
sbit EN_LED = P1^4;

#define LED_LINE P0

unsigned char IS_LED_SWITCHER_ENABLED = 0;

void EnableLedSwitcher();
void TurnOnLed();
void TurnOffLed();


// define BUZZER related members
unsigned int T0_INTERRUPT_CNT = 0;
unsigned int BUZZER_SWITCH_MS = 100;	
unsigned char BUZZER_SWITCH_FLAG = 0;
sbit BUZZ = P1^6;

void ConfigT0();
void TurnOnBuzzer();
void TurnOffBuzzer();
void SwitchBuzzer();

void main() {
	EA = 1;
	ConfigUART(9600);
	ConfigT0();

	while (1) {
		SignalDriver();
	}
}

void ConfigUART(unsigned int baud) {
	// SCON 0101 0000
	// SM1=1, REN=1, ����ͨ��ģʽ
	SCON = 0x50;
	TMOD &= 0x0F;
	TMOD |= 0x20;
	TH1 = 256 - (11059200/12/2/16)/baud;
	TL1 = TH1;
	ET1 = 0; // ʹ��T1��Ϊ�����ʷ�����������ֹT1������ʱ��
	ES = 1; // ʹ�ܴ����ж�
	TR1 = 1; // start T1
}

void SignalDriver() {
	switch (RECEIVED_SIGNAL) {
		case TURN_ON_LED:
			TurnOnLed();
			break;
		case TURN_OFF_LED:
			TurnOffLed();
			break;
		case TURN_ON_BUZZER:
			TurnOnBuzzer();
			break;
		case TURN_OFF_BUZZER:
			TurnOffBuzzer();
			break;
		default:
			break;
	}
}

void InterruptUART() interrupt 4 {
	// �����ж�
	if (RI) {
		RI = 0;
		RECEIVED_SIGNAL = SBUF;
		SBUF = SBUF;
	}
	// �����ж�
	if (TI) {
		TI = 0;
	}
}

void EnableLedSwitcher() {
	// 74HC138 оƬ��ʹ�����ţ�G1 �ߵ�ƽ G2 �͵�ƽ ��������74HC138�� 3-8 �����·
  	ADDR_3 = 1; // G1 �ߵ�ƽ
  	EN_LED = 0; // G2�͵�ƽ��G2A, G2B��
	
	// 110 LEDS6 Ϊ�͵�ƽ�������ܵ�ͨ��LED �ܿ��ش�
  	ADDR_2 = 1;
  	ADDR_1 = 1;
  	ADDR_0 = 0;
}

void TurnOnLed() {
	if (!IS_LED_SWITCHER_ENABLED) {
		EnableLedSwitcher();
		IS_LED_SWITCHER_ENABLED = 1;
	}
	LED_LINE = 0x00;
}

void TurnOffLed() {
	LED_LINE = 0xFF;
}

void ConfigT0() {
	TMOD &= 0xF0;
	TMOD |= 0x01;
	// config T0 interrupt interval 1ms 
	TH0 = 0xFC;
	TL0 = 0x67;
	ET0 = 1; // enalbe T0
}

void InterruptT0() interrupt 1 {
	// reload T0
	TH0 = 0xFC;
	TL0 = 0x67;
	T0_INTERRUPT_CNT++;
	if (T0_INTERRUPT_CNT >= BUZZER_SWITCH_MS) {
		T0_INTERRUPT_CNT = 0;
		BUZZER_SWITCH_FLAG = 1;
	}
}

void TurnOnBuzzer() {
	if (!TR0) {
		TR0 = 1; // start T0
	}
	if (BUZZER_SWITCH_FLAG) {
		BUZZER_SWITCH_FLAG = 0;
		SwitchBuzzer();
	}
}

void TurnOffBuzzer() {
	TR0 = 0; // stop T0
}

void SwitchBuzzer() {
	BUZZ ^= 0x01;
}




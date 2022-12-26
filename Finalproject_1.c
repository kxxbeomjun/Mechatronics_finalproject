#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>

#define LOOPTIME 10 //ms
#define PI 3.141592

#define MOTOR_1 27
#define MOTOR_2 22

#define ENCODER_A 24
#define ENCODER_B 23
#define ENC2REDGEAR 216

#define LED_R 13
#define LED_G 19
#define LED_Y 26

#define SWITCH_1 16
#define SWITCH_2 20
#define SWITCH_3 21

#define PGAIN 220
#define IGAIN 0.0005
#define DGAIN 2

int encA;
int encB;
int N;
int encoderPosition = 0;
float redGearPosition = 0;

float referencePosition;
float errorPosition = 0;
float interrorPosition = 0;
float derierrorPosition = 0;
float beforePosition = 0;

unsigned int startTime;
unsigned int checkTime;
unsigned int checkTimeBefore;

void funcEncoderA()
{
	encA = digitalRead(ENCODER_A);
	encB = digitalRead(ENCODER_B);

	if (encA == HIGH)
	{
		if (encB == LOW)
			encoderPosition--;
		else
			encoderPosition++;
	}
	else
	{
		if (encB == LOW)
			encoderPosition++;
		else
			encoderPosition--;
	}

	redGearPosition = (float)encoderPosition / ENC2REDGEAR * (4 / N);
}

void funcEncoderB()
{
	encA = digitalRead(ENCODER_A);
	encB = digitalRead(ENCODER_B);

	if (encB == HIGH)
	{
		if (encA == LOW)
			encoderPosition++;
		else
			encoderPosition--;
	}
	else
	{
		if (encA == LOW)
			encoderPosition--;
		else
			encoderPosition++;
	}

	redGearPosition = (float)encoderPosition / ENC2REDGEAR * (4 / N);
}

int main()
{
	wiringPiSetupGpio();

	pinMode(ENCODER_A, INPUT);
	pinMode(ENCODER_B, INPUT);

	pinMode(SWITCH_1, INPUT);
	pinMode(SWITCH_2, INPUT);
	pinMode(SWITCH_3, INPUT);

	pinMode(LED_R, OUTPUT);
	pinMode(LED_G, OUTPUT);
	pinMode(LED_Y, OUTPUT);

	softPwmCreate(MOTOR_1, 0, 100);
	softPwmCreate(MOTOR_2, 0, 100);

	int Loop_num = 1;
	startTime = millis();
	checkTimeBefore = millis();

	while (1)
	{
		if(digitalRead(SWITCH_1) == HIGH)
		{
			checkTime = millis();
			if (checkTime - checkTimeBefore > LOOPTIME) {
				float t = 0.001 * LOOPTIME * (Loop_num - 1);
				referencePosition = sin(0.4 * PI * t);
				errorPosition = referencePosition - redGearPosition;
				if (errorPosition > 0)
				{
					softPwmWrite(MOTOR1, 0);
					softPwmWrite(MOTOR2, fabs(errorPosition * PGAIN + interrorPosition * IGAIN + derierrorPosition * DGAIN));
				}
				else
				{
					softPwmWrite(MOTOR1, fabs(errorPosition * PGAIN + interrorPosition * IGAIN + derierrorPosition * DGAIN));
					softPwmWrite(MOTOR2, 0);
				}
				checkTimeBefore = checkTime;
				interrorPosition += errorPosition * LOOPTIME;
				derierrorPosition = (errorPosition - beforePosition) / LOOPTIME;
				beforePosition = errorPosition;
				fprintf(fp, "%d\t%f\n", Loop_num, redGearPosition);
				printf("redGearPosition : %f\n", redGearPosition);
				Loop_num++;
			}

		}
		else if (digitalRead(SWITCH_2) == HIGH) 
		{

		}
		else if (digitalRead(SWITCH_3) == HIGH) 
		{

		}
	}
}
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <math.h>
#include <stdlib.h>

#define LOOPTIME 1
#define PI 3.141592

#define MOTOR1 22
#define MOTOR2 27

#define ENCODER_A 23
#define ENCODER_B 24
#define ENCODER_C 16
#define ENCODER_D 12
#define ENC2REDGEAR 216

#define LED_R 13
#define LED_G 19
#define LED_Y 26

#define SWITCH_1 20
#define SWITCH_2 21

//Encoder difference not considered, fine-tuning needed
int encA;
int encB;
int encC;
int encD;
int encoderPosition = 0;
int encoderPosition2 = 0;

float redGearPosition = 0;
float redGearPosition2 = 0;


float referencePosition;
float errorPosition = 0;
float interrorPosition = 0;
float derierrorPosition = 0;
float beforePosition = 0;
float ITAE = 0;

unsigned int startTime;
unsigned int checkTime;
unsigned int checkTimeBefore = 0;
float t;

float Location[15000] = { 0 };
float Location_imitated[15000] = { 0 };
float MotorVel = 0;

int k = 0;
int cnt = 0; //for record
int ind = 0; //for imitation
int flag = 0;
int Loop_num = 1;

float PGAIN;
float IGAIN;
float DGAIN;

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

    redGearPosition = (float)encoderPosition / ENC2REDGEAR;
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

    redGearPosition = (float)encoderPosition / ENC2REDGEAR;
}
void funcEncoderC()
{
    encC = digitalRead(ENCODER_C);
    encD = digitalRead(ENCODER_D);

    if (encC == HIGH)
    {
        if (encD == LOW)
            encoderPosition2--;
        else
            encoderPosition2++;
    }
    else
    {
        if (encD == LOW)
            encoderPosition2++;
        else
            encoderPosition2--;
    }

    redGearPosition2 = (float)encoderPosition2 / ENC2REDGEAR;

}void funcEncoderD()
{
    encC = digitalRead(ENCODER_C);
    encD = digitalRead(ENCODER_D);

    if (encC == HIGH)
    {
        if (encD == LOW)
            encoderPosition2++;
        else
            encoderPosition2--;
    }
    else
    {
        if (encD == LOW)
            encoderPosition2--;
        else
            encoderPosition2++;
    }

    redGearPosition2 = (float)encoderPosition2 / ENC2REDGEAR;
}

int main(void) {
    wiringPiSetupGpio();

    pinMode(ENCODER_A, INPUT);
    pinMode(ENCODER_B, INPUT);
    pinMode(ENCODER_C, INPUT);
    pinMode(ENCODER_D, INPUT);

    pinMode(SWITCH_1, INPUT);
    pinMode(SWITCH_2, INPUT);

    pinMode(LED_R, OUTPUT);
    pinMode(LED_G, OUTPUT);
    pinMode(LED_Y, OUTPUT);

    softPwmCreate(MOTOR1, 0, 50);
    softPwmCreate(MOTOR2, 0, 50);
    digitalWrite(LED_R, LOW);
    digitalWrite(LED_G, LOW);
    digitalWrite(LED_Y, LOW);

    wiringPiISR(ENCODER_A, INT_EDGE_BOTH, funcEncoderA);
    wiringPiISR(ENCODER_B, INT_EDGE_BOTH, funcEncoderB);
    wiringPiISR(ENCODER_C, INT_EDGE_BOTH, funcEncoderC);
    wiringPiISR(ENCODER_D, INT_EDGE_BOTH, funcEncoderD);
    FILE* fp1 = fopen("/home/pi/Desktop/data_rec.txt", "w");
    fclose(fp1);
    FILE* fp2 = fopen("/home/pi/Desktop/data_imi_imi.txt", "w");
    fclose(fp2);
    FILE* fp3 = fopen("/home/pi/Desktop/fuck.txt", "w");
    fclose(fp3);
    FILE* fp5 = fopen("/home/pi/Desktop/data_imi.txt", "w");
    fclose(fp5);

    while (1) {

        //record
        if (digitalRead(SWITCH_1) == HIGH) {
            digitalWrite(LED_Y, HIGH);

            startTime = millis();
            Loop_num = 1;
            while (1) {
                checkTime = millis();
                if (checkTime - startTime > Loop_num * LOOPTIME)
                {
                    Location[cnt] = redGearPosition; // redGearPosition2
                    FILE* fp1 = fopen("/home/pi/Desktop/data_rec.txt", "a");
                    fprintf(fp1, "%d\t%f\n", cnt, redGearPosition);
                    fclose(fp1);
                    cnt++;
                    printf("Record - loop num: %d encoderposition: %d\n", cnt, encoderPosition); //encoderPosition2
                    Loop_num++;
                }
                if (checkTime - startTime >= 15000)
                {
                    break;
                }
            }
            digitalWrite(LED_Y, LOW);
            cnt = 0;
            redGearPosition = 0; //redGearPosition2
            encoderPosition = 0; //encoderPosition2
        }

        //Imitation
        if (digitalRead(SWITCH_2) == HIGH)
        {
            encoderPosition = 0;
            redGearPosition = 0;
            interrorPosition = 0;
            derierrorPosition = 0;
            beforePosition = 0;
            Loop_num = 1;

            PGAIN = 700;
            IGAIN = 0.005;
            DGAIN = 150;
            digitalWrite(LED_G, HIGH);
            startTime = millis();

            while (1)
            {
                checkTime = millis();
                if (checkTime - startTime > Loop_num * LOOPTIME)
                {
                    referencePosition = Location[ind];
                    if (flag >= 2) {
                        FILE* fp2 = fopen("/home/pi/Desktop/data_imi_imi.txt", "a");
                        fprintf(fp2, "%d\t%f\n", ind, redGearPosition);
                        fclose(fp2);
                    }
                    if (flag == 1) {
                        Location_imitated[ind] = redGearPosition;
                        FILE* fp5 = fopen("/home/pi/Desktop/data_imi.txt", "a");
                        fprintf(fp5, "%d\t%f\n", ind, redGearPosition);
                        fclose(fp5);
                    }
                    if (flag == 0) {

                    }
                    t = (float)(checkTime - startTime) * 0.001;
                    errorPosition = referencePosition - redGearPosition;
                    ITAE += (0.001) * (float)(checkTime - checkTimeBefore) * t * fabs(Location[ind] - redGearPosition);
                    printf("ITAE: %f\n", ITAE);
                    printf("errorposition: %f\n", fabs(Location_imitated[ind] - redGearPosition));

                    if (flag >= 2 && 1000 < ind && ind < 15000 - 500 && fabs(Location_imitated[ind] - redGearPosition)>0.03)//if (1000<ind && ind<14500 && fabs(MotorVel*0.0001 - VelRed[k-1])>0.008)
                    {

                        digitalWrite(LED_G, LOW);
                        digitalWrite(LED_R, HIGH);
                        if (errorPosition > 0)
                        {
                            softPwmWrite(MOTOR1, 8);
                            softPwmWrite(MOTOR2, 0);
                            delay(250);
                        }
                        else
                        {
                            softPwmWrite(MOTOR1, 0);
                            softPwmWrite(MOTOR2, 8);
                            delay(250);
                        }

                        softPwmWrite(MOTOR1, 0);
                        softPwmWrite(MOTOR2, 0);
                        delay(2000);
                        digitalWrite(LED_R, LOW);
                        ind = 0;
                        k = 0;
                        Loop_num = 1;
                        break;
                    }

                    MotorVel = fabs(errorPosition * PGAIN + interrorPosition * IGAIN + derierrorPosition * DGAIN);
                    if (errorPosition > 0)
                    {
                        softPwmWrite(MOTOR1, 0);
                        softPwmWrite(MOTOR2, MotorVel);
                    }
                    else
                    {
                        softPwmWrite(MOTOR1, MotorVel);
                        softPwmWrite(MOTOR2, 0);
                    }
                    printf("output power: %f\n", fabs(errorPosition * PGAIN + interrorPosition * IGAIN + derierrorPosition * DGAIN));
                    checkTimeBefore = checkTime;
                    interrorPosition += errorPosition * LOOPTIME;
                    derierrorPosition = (errorPosition - beforePosition) / LOOPTIME;
                    beforePosition = errorPosition;

                    FILE* fp3 = fopen("/home/pi/Desktop/fuck.txt", "a");
                    fprintf(fp3, "%d\t%f\t%f\n", ind, MotorVel, errorPosition);
                    fclose(fp3);

                    ind++;
                    Loop_num++;

                }
                if (checkTime - startTime >= 15000)
                {
                    break;
                }

            }
            digitalWrite(LED_G, LOW);
            printf("End of Imitation\n");

            encoderPosition = 0;
            redGearPosition = 0;

            errorPosition = 0;
            interrorPosition = 0;
            derierrorPosition = 0;
            beforePosition = 0;

            ind = 0;
            k = 0;
            ITAE = 0;
            printf("#############################\n");
            softPwmWrite(MOTOR1, 0);
            softPwmWrite(MOTOR2, 0);
            flag++;
        }

    }
    return 0;
}
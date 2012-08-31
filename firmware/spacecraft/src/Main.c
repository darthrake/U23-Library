#include <stdint.h>
#include <string.h>

#include <RCC.h>
#include <System.h>
#include <LED.h>
#include <VGA.h>
#include <Random.h>

#include <arm_math.h>
#include <Accelerometer.h>
#include <Button.h>
#include <Sprites.h>

void Delay(uint32_t time);
static uint32_t sqrti(uint32_t n);
static int8_t zero[3];
static int32_t acc_x=0,acc_y=0;

int main()
{
	InitializeSystem();
	SysTick_Config(HCLKFrequency()/100);
	InitializeLEDs();

	SetLEDs(0x01);

	uint8_t *framebuffer1=(uint8_t *)0x20000000;
	uint8_t *framebuffer2=(uint8_t *)0x20010000;
	SetLEDs(0x03);
	memset(framebuffer1,0,320*200);
	memset(framebuffer2,0,320*200);

	SetLEDs(0x07);

	IntializeVGAScreenMode320x200(framebuffer1);
	InitializeAccelerometer();

	SetAccelerometerMainConfig(
		LIS302DL_LOWPOWERMODE_ACTIVE|
		LIS302DL_DATARATE_100|
		LIS302DL_XYZ_ENABLE|
		LIS302DL_FULLSCALE_2_3|
		LIS302DL_SELFTEST_NORMAL);

	//Wait one second for data to stabilize
	Delay(100);

	SetAccelerometerFilterConfig(
		LIS302DL_FILTEREDDATASELECTION_BYPASSED|
		LIS302DL_HIGHPASSFILTER_LEVEL_1|
		LIS302DL_HIGHPASSFILTERINTERRUPT_1_2);

	Delay(50);
	ReadRawAccelerometerData(zero);

	#define NumberOfStars 12
	static struct Star
	{
		int x,y,dy,f;
	} stars[NumberOfStars];

	for(int i=0;i<NumberOfStars;i++)
	{
		stars[i].x=RandomInteger()%360;
		stars[i].y=(RandomInteger()%200);

		int z=sqrti((NumberOfStars-1-i)*NumberOfStars)*1000/NumberOfStars;
		stars[i].dy=1;//6000*1200/(z+200);

		stars[i].f=(6-(z*7)/1000)+(RandomInteger()%6)*7;
	}

	const RLEBitmap *sprites[7*6]={
		&Star1_0,&Star2_0,&Star3_0,&Star4_0,&Star5_0,&Star6_0,&Star7_0,
		&Star1_1,&Star2_1,&Star3_1,&Star4_1,&Star5_1,&Star6_1,&Star7_1,
		&Star1_2,&Star2_2,&Star3_2,&Star4_2,&Star5_2,&Star6_2,&Star7_2,
		&Star1_3,&Star2_3,&Star3_3,&Star4_3,&Star5_3,&Star6_3,&Star7_3,
		&Star1_4,&Star2_4,&Star3_4,&Star4_4,&Star5_4,&Star6_4,&Star7_4,
		&Star1_5,&Star2_5,&Star3_5,&Star4_5,&Star5_5,&Star6_5,&Star7_5,
	};

	const RLEBitmap *spacecraft[3]={
		&Space_0_0, &Space_0_1, &Space_0_2
	};

	Bitmap frame1,frame2;
	InitializeBitmap(&frame1,320,200,320,framebuffer1);
	InitializeBitmap(&frame2,320,200,320,framebuffer2);

	int frame=0;

	int pos_x = (320-42)/2;
	int pos_y = (200-42)/2;

	while(1)
	{
		WaitVBL();

		Bitmap *currframe;
		if(frame&1) { currframe=&frame2; SetFrameBuffer(framebuffer1); }
		else { currframe=&frame1; SetFrameBuffer(framebuffer2); }

		ClearBitmap(currframe);

		for(int i=0;i<NumberOfStars;i++)
		{
			DrawRLEBitmap(currframe,sprites[stars[i].f],(stars[i].x),stars[i].y);

			stars[i].y+=stars[i].dy;
			if(stars[i].y>=(200))
			{
				stars[i].y=(0);
				stars[i].x=RandomInteger()%320;
				stars[i].f=(stars[i].f%7)+(RandomInteger()%6)*7;
			}
		}


		if (UserButtonState()) {
			ReadRawAccelerometerData(zero);
		}

		int dir = 0;

		int8_t components[3];
		ReadRawAccelerometerData(components);

		int32_t dx=components[1]-zero[1];
		int32_t dy=components[0]-zero[0];

		dx = (dx / 10) * 10;
		dy = (dy / 10) * 10;

		if (dx < 0)
		{
			dir = 1;
		} else if (dx > 0)
		{
			dir = -1;
		}

		pos_x -= dx/10;
		pos_y -= dy/10;

		if (pos_x < 0)
		{
			pos_x = 0;
		}

		if (pos_x >= 320)
		{
			pos_x = 320;
		}

		if (pos_y >= 200)
		{
			pos_y = 200;
		}

		if (pos_y < 0)
		{
			pos_y = 0;
		}

		DrawRLEBitmap(currframe, spacecraft[dir+1], pos_x, pos_y);

		frame++;
	}
}


volatile uint32_t SysTickCounter=0;

void Delay(uint32_t time)
{
	uint32_t end=SysTickCounter+time;
	while(SysTickCounter!=end);
}

void SysTick_Handler()
{  
	SysTickCounter++;
}

static uint32_t sqrti(uint32_t n)
{
	uint32_t s,t;

	#define sqrtBit(k) \
	t = s+(1UL<<(k-1)); t <<= k+1; if (n >= t) { n -= t; s |= 1UL<<k; }

	s=0;
	if(n>=1<<30) { n-=1<<30; s=1<<15; }
	sqrtBit(14); sqrtBit(13); sqrtBit(12); sqrtBit(11); sqrtBit(10);
	sqrtBit(9); sqrtBit(8); sqrtBit(7); sqrtBit(6); sqrtBit(5);
	sqrtBit(4); sqrtBit(3); sqrtBit(2); sqrtBit(1);
	if(n>s<<1) s|=1;

	#undef sqrtBit

	return s;
}

void NMI_Handler()
{
	for(;;);
}

void HardFault_Handler()
{
	for(;;);
}

void BusFault_Handler()
{
	for(;;);
}

void UsageFault_Handler()
{
	for(;;);
}

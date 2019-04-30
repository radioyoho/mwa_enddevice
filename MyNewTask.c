/*
 * MyNewTask.c
 *
 *  Created on: 9 abr. 2019
 *      Author: aland
 */

#include "MyNewTask.h"
#include "fsl_os_abstraction.h"
#include "mwa_end_device.h"

void My_Task(osaTaskParam_t argument);
static void myTaskTimerCallback(void *param);

osaEventId_t mMyEvents;
/* Global Variable to store our TimerID */
tmrTimerID_t myTimerID = gTmrInvalidTimerID_c;
/* Handler ID for task */
osaTaskId_t gMyTaskHandler_ID;
/* Local variable to store the current state of the LEDs */
static uint8_t ledsState = 0;

/* OSA Task Definition*/
OSA_TASK_DEFINE(My_Task, gMyTaskPriority_c, 1, gMyTaskStackSize_c, FALSE );

/* Main custom task */
void My_Task(osaTaskParam_t argument)
{
	uint8_t msg[13] = "Counter: x\n\r";
	msg[12] = 0;

	uint8_t counter = 255;

	osaEventFlags_t customEvent;
	myTimerID = TMR_AllocateTimer();
	while(1)
	{
		OSA_EventWait(mMyEvents, osaEventFlagsAll_c, FALSE, osaWaitForever_c,
		&customEvent);

		if( !gUseRtos_c && !customEvent)
		{
			break;
		}

		/* Depending on the received event */
		switch(customEvent)
		{
			case gMyNewTaskEvent1_c:
				TMR_StartIntervalTimer(myTimerID, /*myTimerID*/
				3000, /* Timer's Timeout */
				myTaskTimerCallback, /* pointer to
				myTaskTimerCallback function */
				NULL
				);

				TurnOffLeds(); /* Ensure all LEDs are turned off */
				break;

			case gMyNewTaskEvent2_c: /* Event called from myTaskTimerCallback */

				TurnOffLeds();
				counter = ((counter + 1) % 4);
				LED_Operate(1 << counter, gLedOn_c);
				msg[SizeOfString(msg) - 3] = '1' + counter;
				App_TransmitData(&msg[0], SizeOfString(msg));

				break;
			case gMyNewTaskEvent3_c: /* Event to stop the timer */
				ledsState = 0;
				TurnOffLeds();
				TMR_StopTimer(myTimerID);
				break;
			default:
				break;
		}
	}
}

/* Function to init the task */
void MyTask_Init(void)
{
	mMyEvents = OSA_EventCreate(TRUE);
	/* The instance of the MAC is passed at task creaton */
	gMyTaskHandler_ID = OSA_TaskCreate(OSA_TASK(My_Task), NULL);
}

/* This is the function called by the Timer each time it expires */
static void myTaskTimerCallback(void *param)
{
	OSA_EventSet(mMyEvents, gMyNewTaskEvent2_c);
}

/* Public function to send an event to stop the timer */
void MyTaskTimer_Stop(void)
{
	OSA_EventSet(mMyEvents, gMyNewTaskEvent3_c);
}

/* Public function to send an event to stop the timer */
void MyTaskTimer_Start(void)
{
	OSA_EventSet(mMyEvents, gMyNewTaskEvent1_c);
}

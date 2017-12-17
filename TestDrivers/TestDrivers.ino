//////////
// Includes

#include <HBLib_Encoder.h>

//
//////////

//////////
// Name space definition 
//

using namespace HessBay::Library::Components;

//
//////////

//////////
// Constants definition.

// Pin A of the encoder.  Must be a digital pin.
const int PIN_ENCODER_A = 2;

// Pin B of the encoder.  Must be a digital pin.
const int PIN_ENCODER_B = 3;

// Specify if the high resolution mode is active.
const bool ENCODER_HIGHRES = false;

// Activate the high speed mode if the delay between two value changes of the encoder is less than the specified delay in ms.
const int ENCODER_HIGHSPEED_TRIGGER_MS = 70;

// When in high speed mode, each encoder value change will be multiplied by this factor.
const int ENCODER_HIGHSPEED_MULTIPLIER = 5;

//
//////////

//////////
// Global variables definition.

// Encoder class under test.
Encoder _encoder = Encoder(
	PIN_ENCODER_A,
	PIN_ENCODER_B,
	ENCODER_HIGHRES,
	true,
	ENCODER_HIGHSPEED_TRIGGER_MS,
	ENCODER_HIGHSPEED_MULTIPLIER,
	HandleEncoderInterrupt);

// Cumulative count of encoder value changes.
volatile int _encoderCount = 0;

//
//////////

void setup()
{
	Serial.begin(115200);
}

void loop()
{
}

/////
// Handle rotary encoder interrupts who occurs on value changes.
/////
void HandleEncoderInterrupt()
{
	int delta = _encoder.DeltaValue();
	if (delta != 0)
	{
		_encoderCount += delta;
		Serial.println(_encoderCount);
	}
}
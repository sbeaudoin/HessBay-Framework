﻿HessBay Library - **Components** 

# Encoder
This class drive a rotary encoder.  It uses the defined pins for the A and B channel of the encoder.  As the pins can be 
configured to use the internal pull-up resistors, they are optional.  

The consumer of this class must declare and interrupt on pin change of the A and B channels.  In the interrupt method, the 
'ReadDelta' method of this class should be called to get the new value.

## Technical notes
###	Encoder pins signal

Considering the two signal pins of the rotary encoder pin A and pin B.  Depending of the way you connected those pins to the 
encoder, a clockwise rotation will produce the following signal.

![Clockwise Signal](Documentation/Images/Encoder_Signal_Clockwise.png?raw=true)

A counter clockwise rotation will produce the following signal.

![Counter Clockwise Signal](Documentation/Images/Encoder_Signal_CounterClockwise.png?raw=true)

In a forward motion, the pin A signal follow the start of pin B by half a cycle.  In a reverse motion, pin A **precede** the 
start of pin B by half a cycle.

Here is the pins used for this documentation.

![Encoder Pins Used](Documentation/Images/Encoder_Pins.png?raw=true)

### Transitions resolution

#### High resolution
It is possible to follow the encoder progress with full resolution by checking each signal transitions.  This gives 16 different
transitions if we include the start and end of the signal who does not really indicate a change.  In this mode, each click will
gives four transitions and so, four value changes.

For each transition, a state value if deducted.  This state is in a 4 bit forms, each one correspond to a signal state at a 
specific point in time.  Here is a truth table for each possible transitions by state value.

|Previous (A/B)  |New (A/B)  |Type      |Direction          |Delta Value              |State Value  |
|---|---|---|---|---|---|
| L/L            | L/L       |Stationary|None               | +0                      | 0000 = 0    |
| L/L            | L/H       |Normal    |Clockwise          | +1                      | 0001 = 1    |
| L/L            | H/L       |Normal    |Counter            | -1                      | 0010 = 2    |
| L/L            | H/H       |Abnormal  |Clockwise          | +2 (missed one +1 step) | 0011 = 3    |
| L/H            | L/L       |Normal    |Counter            | -1                      | 0100 = 4    |
| L/H            | L/H       |Stationary|None               | +0                      | 0101 = 5    |
| L/H            | H/L       |Abnormal  |Counter            | -2 (missed one -1 step) | 0110 = 6    |
| L/H            | H/H       |Normal    |Clockwise          | +1                      | 0111 = 7    |
| H/L            | L/L       |Normal    |Clockwise          | +1                      | 1000 = 8    |
| H/L            | L/H       |Abnormal  |Counter            | -2 (missed one -1 step) | 1001 = 9    |
| H/L            | H/L       |Stationary|None               | +0                      | 1010 = 10   |
| H/L            | H/H       |Normal    |Counter            | -1                      | 1011 = 11   |
| H/H            | L/L       |Abnormal  |Clockwise          | +2 (missed one +1 step) | 1100 = 12   |
| H/H            | L/H       |Normal    |Counter Clockwise  | -1                      | 1101 = 13   |
| H/H            | H/L       |Normal    |Clockwise          | +1                      | 1110 = 14   |
| H/H            | H/H       |Stationary|None               | +0                      | 1111 = 15   |

The *type* column indicates if this state transition is normal, abnormal or stationary.  A normal transition correspond to one of 
the transitions specified in the two images.  An abnormal transition usually indicates we missed a transition.  In this case, it 
is possible to deduct the missed transition and correct the delta value.  The last state, stationary, occurs when no rotation 
occurred.

The *direction* column indicate the rotation direction associated with this transition.  It can be clockwise, counter clockwise
or none.  

Finally, the *delta value* column indicates the adjustment required to the value changed by the encoder to respect the 
transition.

#### Low resolution

In this mode, we only take care of the transitions who occurs on a click.  This lower the CPU time required, but at a loss of
resolution.  Also, in this mode, we don't compensate for possible missing state transitions.

|Previous (A/B)  |New (A/B)  |Type      |Direction          |Delta Value              |State Value  |
|---|---|---|---|---|---|
| H/L            | L/L       |Normal    |Clockwise          | +1                      | 1000 = 8    |
| H/L            | H/L       |Stationary|None               | +0                      | 1010 = 10   |
| H/L            | H/H       |Normal    |Counter            | -1                      | 1011 = 11   |

## Implementation

#### Hardware

- Pin A and B of the encoder are connected directly to a digital pin of the arduino board.  Except if you want to use your own 
  pull up resistors.
- Ground pin is connected to a ground pin of the arduino board.

#### Software

- [A] : Inclusion of the header file of the library.
- [B] : I use namespaces.  To be able to use the code without having to specify the namespace at each line, the 
        namespace definition is specified here.
- [C] : Define your constants.
- [D] : Initialize your instance.  There is two version of the constructor, depending if you want the library to use interrupts
        or not.  I suggest you use the interrupt driven version to be sure you don't miss any encoder changes.
- [E] : Any variable you use in the callback function should be volatiles.
- [F] : This is an example of the callback function who is called each time the pin A or B state changes.

```
//-----\ [A]
#include <HBLib_Encoder.h>
//-----/ [A]

//-----\ [B]
using namespace HessBay::Library::Components;
//-----/ [B]

//-----\ [C]

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

//-----/ [C]

//-----\ [D]
Encoder _encoder = Encoder(
	PIN_ENCODER_A,
	PIN_ENCODER_B,
	ENCODER_HIGHRES,
	true,
	ENCODER_HIGHSPEED_TRIGGER_MS,
	ENCODER_HIGHSPEED_MULTIPLIER,
	HandleEncoderInterrupt);
//-----/ [D]

//-----\ [E]

// Cumulative count of encoder value changes.
volatile int _encoderCount = 0;

//-----/ [E]

//
//////////

void setup()
{
	Serial.begin(115200);
}

void loop()
{
}

//-----\ [F]

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

//-----/ [F]

```

## Class content

### Constructors

```
Encoder(
    byte pinA, 
    byte pinB, 
    bool highResolution, 
    bool pullUpInternal, 
    int highSpeedTriggerMS, 
    int highSpeedTriggerMultiplier);
Encoder(
    byte pinA, 
    byte pinB, 
    bool highResolution, 
    bool pullUpInternal, 
    int highSpeedTriggerMS, 
    int highSpeedTriggerMultiplier, 
    void(*callBack)(void));
```

### Properties 
No properties are available.

### Functions
```
int DeltaValue();
```

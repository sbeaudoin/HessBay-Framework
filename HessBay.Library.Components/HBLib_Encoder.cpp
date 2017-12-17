#include "HBLib_Encoder.h"

namespace HessBay
{
	namespace Library
	{
		namespace Components
		{
			/////
			// Encoder driver
			// v1.0.0 - 2017.12.17
			// ----
			// See the header file for more details.
			/////


			/////
			// Initialize the instance.  This version must be used if you don't want to use interrupts.
			//
			// pinA 
			// ----
			// Pin number used by the encoder pin A.
			//
			// pinB
			// ----
			// Pin number used by the encoder pin B.
			//
			// highResolution
			// --------------
			// If true, the high resolution mode is active.  In this mode, a single click on the encoder will trigger four
			// value changes.  If false, the normal resolution mode is active.  In this mode, a single click on the encoder
			// will trigger a single value change.
			//
			// pullUpInternal
			// --------------
			// If true, the internal pull-up of the pins used for pin A and B will be activated.  If you prefer to use real
			// pull up resistors, assign false to this parameter.
			// 
			// highSpeedTriggerMS
			// ------------------
			// The fast mode will be activated if two value changes occur within this specified delay in ms.  In the high
			// resolution mode, this delay is considered only once within the four state changes.
			//
			// highSpeedTriggerMultiplier
			// --------------------------
			// When the fast mode is active, the delta value will be multiplied by this factor.
			/////
			Encoder::Encoder(
				byte pinA, 
				byte pinB, 
				bool highResolution, 
				bool pullUpInternal, 
				int highSpeedTriggerMS, 
				int highSpeedTriggerMultiplier)
				: Encoder::Encoder(pinA, pinB, highResolution, pullUpInternal, highSpeedTriggerMS, highSpeedTriggerMultiplier, NULL)
			{}

			/////
			// Initialize the instance.  This version use interrupts.  
			//
			// pinA 
			// ----
			// Pin number used by the encoder pin A.
			//
			// pinB
			// ----
			// Pin number used by the encoder pin B.
			//
			// highResolution
			// --------------
			// If true, the high resolution mode is active.  In this mode, a single click on the encoder will trigger four
			// value changes.  If false, the normal resolution mode is active.  In this mode, a single click on the encoder
			// will trigger a single value change.
			//
			// pullUpInternal
			// --------------
			// If true, the internal pull-up of the pins used for pin A and B will be activated.  If you prefer to use real
			// pull up resistors, assign false to this parameter.
			// 
			// highSpeedTriggerMS
			// ------------------
			// The fast mode will be activated if two value changes occur within this specified delay in ms.  In the high
			// resolution mode, this delay is considered only once within the four state changes.
			//
			// highSpeedTriggerMultiplier
			// --------------------------
			// When the fast mode is active, the delta value will be multiplied by this factor.
			//
			// callBack
			// --------
			// Function who will be called at each value change of pin A or B.
			/////
			Encoder::Encoder(
				byte pinA, 
				byte pinB, 
				bool highResolution, 
				bool pullUpInternal, 
				int highSpeedTriggerMS, 
				int highSpeedTriggerMultiplier, 
				void(*callBack)(void))
			{
				// Initialize the pins.
				byte inputType = (pullUpInternal) ? INPUT_PULLUP : INPUT;
				pinMode(pinA, inputType);
				pinMode(pinB, inputType);

				// Initialize the interrupts if requested.
				if (callBack != NULL)
				{
					int intrA = digitalPinToInterrupt(pinA);
					attachInterrupt(intrA, callBack, CHANGE);
					int intrB = digitalPinToInterrupt(pinB);
					attachInterrupt(intrB, callBack, CHANGE);
				}

				// Initialize the class variables.
				_pinA = pinA;
				_pinB = pinB;
				_prevA = digitalRead(pinA);
				_prevB = digitalRead(pinB);
				_isHighResolution = highResolution;
				_lastInterrupt = millis();
				_HSTriggerMS = highSpeedTriggerMS;
				_HSTriggerMultiplier = highSpeedTriggerMultiplier;
			}

			/////
			// Compare the pin A and B state with the previous one obtained on the previous call and compute a delta value
			// who will be returned.
			//
			// Returns
			// -------
			// The value change of the encoder since the last call.
			/////
			int Encoder::DeltaValue()
			{
				// Read the new pin states.
				int newA = digitalRead(_pinA);
				int newB = digitalRead(_pinB);

				int delta = 0;

				// Switch to high or normal resolution depending of the activated mode.
				if (_isHighResolution)
				{
					delta = ReadDeltaHighRes(newA, newB);
				}
				else // Normal resolution.  One value change per click.
				{
					delta = ReadDeltaLowRes(newA, newB);
				}

				// Update the old pin state.
				_prevA = newA;
				_prevB = newB;

				return delta;
			}

			/////
			// Compute a delta value in high resolution mode.
			//
			// newA
			// ----
			// Current pin A state.
			// 
			// newB
			// ----
			// Current pin B state.
			//
			// Returns
			// -------
			// The delta value corresponding to the pins state change of the last call.
			/////
			int Encoder::ReadDeltaHighRes(byte newA, byte newB)
			{
				int delta = 0;

				// Only process if a state change occurred on pin A or B.
				if (newA != _prevA || newB != _prevB)
				{
					// Product a state byte with the following binary format.
					//
					//		ABCD
					//		^^^^
					//      |||+-- New channel B state.
					//      ||+--- New channel A state.
					//      |+---- Previous channel B state.
					//      +----- Previous channel A state.
					byte state = 0;
					state = _prevA << 3 | _prevB << 2 | newA << 1 | newB;

					// Find the delta depending of the state change.
					switch (state)
					{
						case 1:  // 00->01 : A is low and unchanged, B changed from low to high.
						case 7:  // 01->11 : A changed from low to high, B is high and unchanged.
						case 8:  // 10->00 : A changed from high to low, B is low and unchanged.
						case 14: // 11->10 : A is high and unchanged, B changed from high to low.
							delta = 1;
							break;

						case 2:  // 00->10 : A changed from low to high, B is low and unchanged.
						case 4:  // 01->00 : A is unchanged and low, B changed from high to low.
						case 11: // 10->11 : A is unchanged and high, B changed from low to high.
						case 13: // 11->01 : A changed from high to low, B is unchanged and high.
							delta = -1;
							break;

						case 3:  // 00->11 : A and B changed from low to high.
						case 12: // 11->00 : A and B changed from high to low.

								 // We missed a clockwise step and received a clockwise step.
							delta = 2;
							break;

						case 6: // 01->10 : A changed from low to high, B changed from high to low.
						case 9: // 10->01 : A changed from high to low, B changed from low to high.

								// We missed a counter-clockwise step and received a counter-clockwise step.
							delta = -2;
							break;
					}

					// Handle the high speed multiplier on only one of the trigger points.  Otherwise, each click will trigger 
					// four false high speed changes.
					if (newA == HIGH && newB == HIGH)
					{
						delta = HandleDeltaMultiplier(delta);
					}
				}
					
				return delta;
			}

			/////
			// Compute a delta value in low (normal) resolution mode.
			//
			// newA
			// ----
			// Current pin A state.
			// 
			// newB
			// ----
			// Current pin B state.
			//
			// Returns
			// -------
			// The delta value corresponding to the pins state change of the last call.
			/////
			int Encoder::ReadDeltaLowRes(byte newA, byte newB)
			{
				int delta = 0;

				// A value change is produced when pin A goes from high to low.
				if (_prevA == HIGH && newA == LOW)
				{
					// A low state on pin B indicates a clockwise rotation.  If not, an anti clockwise rotation occured.
					if (newB == LOW)
					{
						delta = 1;
					}
					else // B is HIGH
					{
						delta = -1;
					}
				}

				// Handle the fast mode if required.
				delta = HandleDeltaMultiplier(delta);

				return delta;
			}

			/////
			// Detect if the fast mode factor should be applied and if so, apply it.
			//
			// delta
			// -----
			// The real delta changed produced by the encoder.
			//
			// Returns
			// -------
			// The real delta change if the fast mode is not active.  If the fast mode is active, this will return the real
			// delta changed multiplied by the fast mode factor.
			/////
			int Encoder::HandleDeltaMultiplier(int delta)
			{
				int newDelta = delta;

				if (delta != 0)
				{
					// If the last request occurred less than the multiplier trigger, we multiply by the defined constant.
					unsigned long newInterrupt = millis();
					int deltaT = newInterrupt - _lastInterrupt;
					if (deltaT <= _HSTriggerMS)
					{
						newDelta *= _HSTriggerMultiplier;
					}

					_lastInterrupt = newInterrupt;
				}

				return newDelta;
			}
		}
	}
}


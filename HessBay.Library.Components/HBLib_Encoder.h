#ifndef _HBLib_Encoder
#define _HBLib_Encoder
#endif	
#pragma once

#include <Arduino.h>

namespace HessBay
{
	namespace Library
	{
		namespace Components
		{
			/////
			// Encoder driver
			// v1.0.0 - 2017.12.17
			//
			// ----- Description -----
			// Drive an encoder to read the relative value of a partial rotation.  The change is interrupt driven to be sure to
			// not miss any changes.  A fast mode is supported.  In this mode, the rate of change is multiplied by a specified
			// constant.
			//
			// ----- Usage -----
			// The details of the implementation can be found at the gitHub repository at the following link :
			// https://github.com/sbeaudoin/HessBay-Framework.
			//
			// In brief, the encoder have a three pin section.  If those three pins are directly in front of you, the left one
			// is pin A, the center one is ground and the right one is pin B.  The A and B pin should be connected directly to 
			// any digital pin of the board.  The ground pin should be connected to a ground pin of the board.
			//
			// The sketch can have a parameterless void function who will be called at each encoder signal change.  This 
			// function should call the DeltaValue method of the encoder to receive a delta value depending of the direction of
			// the rotation.  This value will be multiplied by the specified factor is the fast mode is active.  Since the 
			// callback method will be called from an interrupt, any values modified in this function must be marked volatile.
			//
			// You can also use this class without a callback function.  In this case, you should call DeltaValue in the loop
			// section, but be aware you can possibly miss some values.
			/////
			class Encoder
			{
			public:

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
				Encoder(
					byte pinA, 
					byte pinB, 
					bool highResolution, 
					bool pullUpInternal, 
					int highSpeedTriggerMS, 
					int highSpeedTriggerMultiplier);

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
				Encoder(
					byte pinA, 
					byte pinB, 
					bool highResolution, 
					bool pullUpInternal, 
					int highSpeedTriggerMS, 
					int highSpeedTriggerMultiplier, 
					void(*callBack)(void));

				/////
				// Compare the pin A and B state with the previous one obtained on the previous call and compute a delta value
				// who will be returned.
				//
				// Returns
				// -------
				// The value change of the encoder since the last call.
				/////
				int DeltaValue();

			private:

				// Pin number for the A signal.
				byte _pinA;

				// Pin number for the B signal.
				byte _pinB;

				// State of the A pin on the previous call.
				byte _prevA;

				// State of the B pin on the previous call.
				byte _prevB;

				// Indicate if the high resolution mode is enabled.
				bool _isHighResolution;

				// The time stamp of the last time a value change occured.
				volatile unsigned long _lastInterrupt;

				// The maximum number of milliseconds who should occur between two value changes of the encoder to activate the
				// fast mode.
				int _HSTriggerMS;

				// The multiplier who should be applied to the value change when fast mode is enabled.
				int _HSTriggerMultiplier;

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
				int ReadDeltaHighRes(byte newA, byte newB);

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
				int ReadDeltaLowRes(byte newA, byte newB);

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
				int HandleDeltaMultiplier(int delta);
			};
		}
	}
}
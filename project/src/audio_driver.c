#include "stm32f4xx.h"
#include "audio_driver.h"
#include "stm32f4_discovery_audio_codec.h"
#include "led.h"
#include "motor.h"

//-------------------------------------------------------------------

int16_t prevFreq = -1;

//-------------------------------------------------------------------

const uint16_t AUDIO_SAMPLE[48] = { 0 };	// Stilte

//-------------------------------------------------------------------

void Project_Init (void)
{
	GPIO_InitTypeDef  gpioInit;
	
	// Initialize Audio interface
  EVAL_AUDIO_SetAudioInterface( AUDIO_INTERFACE_I2S );
	EVAL_AUDIO_Init( OUTPUT_DEVICE_BOTH, 60, I2S_AudioFreq_48k );		// Volume 80%, 44Khz samplefrequentie
	Audio_MAL_Play((uint32_t)AUDIO_SAMPLE, sizeof(AUDIO_SAMPLE) );	// Speel stilte, waarover de toongenerator de toon speelt
	EVAL_AUDIO_PauseResume(AUDIO_RESUME);														// Start met spelen

	// Initialiseer overige audio registers
	//Codec_WriteRegister(0x04, 0xAA);	// Heaphone and speaker amplifier always on
	//Codec_WriteRegister(0x09, 0x0F);	// Passthrough Input Channel Mapping
	//Codec_WriteRegister(0x0D, 0xD0);  // Playback Control 1 
	//Codec_WriteRegister(0x0E, 0x00);  // Miscellaneous Controls
	//Codec_WriteRegister(0x20, 0x18);  // Master Volume Control A
	//Codec_WriteRegister(0x21, 0x18);  // Master Volume Control B
	//Codec_WriteRegister(0x22, 0xFF);  // Headphone Volume Control A
	//Codec_WriteRegister(0x23, 0xFF);  // Headphone Volume Control B
	//Codec_WriteRegister(0x24, 0xFF);  // Speaker Volume Control A
	//Codec_WriteRegister(0x25, 0xFF);  // Speaker Volume Control B
	
	// Initialiseer Beep generator
	//Codec_WriteRegister(0x1C, 0x01);
	//Codec_WriteRegister(0x1D, 0x00);
	//Codec_WriteRegister(0x1E, 0x60);
	
	// Motor init
	motor_init();
	setLed(1);
	
	/*
	// Motor test
	// Dit staat hier omdat een van de andere init's niet werkt zonder evdk
	motor_init();
	setLedBlink();
	{int i; for(i=0;i<1000000;i++);}	// Small delay for the motor control board to accept the previous command
	// Motor ��n rondje
	SMC_step(1600,1,1000,1);
	*/
	
}

//-------------------------------------------------------------------

uint8_t Project_PlayTone (const AudioDriver_Tone freq)
{
	uint8_t toneReg = 0x01;

	// Stop als geen nieuwe toon nodig
	if ( (freq <= 0) && (freq != prevFreq) )
	{
		Codec_WriteRegister(0x1E, 0x00);	// Stop huidige toon
		return 0;
	}
	else if ( freq == prevFreq )
	{
		return 0;												// Geen toonverandering nodig
	}
	else
	{
		prevFreq = freq;
	}

	// Bereken register waarde, voor toon frequentie en aan-tijd
	toneReg |= ((freq > Tone_C7) ? (Tone_C7) : (freq)) << 4;
	
	// Zet benodigde registers voor nieuwe toon
	Codec_WriteRegister(0x1C, toneReg);					// Beep Frequency & aan-tijd
	Codec_WriteRegister(0x1D, 0x03);						// Beep Volume    & uit-Time
	Codec_WriteRegister(0x1E, Beep_Continuous);	// Beep generator aan, produceer enkele toon

	return freq;
}


//-------------------------------------------------------------------

void Project_StartMotor (void)
{
	const char* startString = "9372498723498";
	
	for( ; !startString; startString++)
  {
    while(USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET); // Wait for Empty
 
    USART_SendData(USART1, *startString); // Send 'I'
  }
}

//-------------------------------------------------------------------

uint32_t Codec_TIMEOUT_UserCallback(void)
{
	led_toggle(LED_ORANGE);
	while (1);
}

//-------------------------------------------------------------------

void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size)
{
	Audio_MAL_Play((uint32_t)AUDIO_SAMPLE, sizeof(AUDIO_SAMPLE) );
	//EVAL_AUDIO_PauseResume(AUDIO_RESUME);
}

//-------------------------------------------------------------------

uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
	return 0;
}
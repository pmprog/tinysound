// only create header symbols
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

// create header + implementation
#define TS_IMPLEMENTATION
#include "tinysound.h"

static tsLoadedSound vorbis_loaded;
static tsPlayingSound vorbis_sound;

void Vorbis( tsContext* ctx )
{
	// make these static as a hacky way to keep them around
	// once this function returns.
	int sample_rate;

	vorbis_loaded = tsLoadOGG( "thingy.ogg", &sample_rate );
	vorbis_sound = tsMakePlayingSound( &vorbis_loaded );
	tsSetVolume( &vorbis_sound, 0.3f, 0.3f );
	tsInsertSound( ctx, &vorbis_sound );
}

void LowLevelAPI( tsContext* ctx )
{
	tsLoadedSound airlock = tsLoadWAV( "airlock.wav" );
	tsLoadedSound jump = tsLoadWAV( "../jump.wav" );
	tsPlayingSound s0 = tsMakePlayingSound( &airlock );
	tsPlayingSound s1 = tsMakePlayingSound( &jump );
	tsInsertSound( ctx, &s0 );

	// Play the vorbis song thingy.ogg, or loop airlock
	if ( 0 ) tsLoopSound( &s0, 1 );
	else Vorbis( ctx );

	while ( 1 )
	{
		if ( GetAsyncKeyState( VK_ESCAPE ) )
			break;

		if ( GetAsyncKeyState( VK_SPACE ) )
			tsInsertSound( ctx, &s1 );

		tsMix( ctx );
	}

	tsFreeSound( &vorbis_loaded );
}

float Time( )
{
	static int first = 1;
	static LARGE_INTEGER prev;

	LARGE_INTEGER count, freq;
	ULONGLONG diff;
	QueryPerformanceCounter( &count );
	QueryPerformanceFrequency( &freq );

	if ( first )
	{
		first = 0;
		prev = count;
	}

	diff = count.QuadPart - prev.QuadPart;
	prev = count;
	return (float)(diff / (double)freq.QuadPart);
}

void HighLevelAPI( tsContext* ctx )
{
	tsLoadedSound airlock = tsLoadWAV( "airlock.wav" );
	tsLoadedSound rupee1 = tsLoadWAV( "LTTP_Rupee1.wav" );
	tsLoadedSound rupee2 = tsLoadWAV( "LTTP_Rupee2.wav" );
	tsPlaySoundDef def0 = tsMakeDef( &airlock );
	tsPlaySoundDef def1 = tsMakeDef( &rupee1 );
	tsPlaySoundDef def2 = tsMakeDef( &rupee2 );
	tsPlaySound( ctx, def0 );

	while ( 1 )
	{
		if ( GetAsyncKeyState( VK_ESCAPE ) )
			break;

		// if space is pressed play rupee noise and start tracking time
		static int debounce = 1;
		static float time = 0.0f;
		if ( GetAsyncKeyState( VK_SPACE ) && debounce )
		{
			tsPlaySound( ctx, def1 );
			debounce = 0;
		}

		// if we pressed space track time, once we wait long enough go
		// ahead nad play the second rupee noise as long as we are
		// holding space
		if ( !debounce )
		{
			time += Time( );
			if ( time > 0.2f )
			{
				tsPlaySound( ctx, def2 );
				time = 0.0f;
			}
		}

		// clear time accumulations when not pressing space
		else Time( );

		// if we pressed space but aren't any longer clear all state
		if ( !GetAsyncKeyState( VK_SPACE ) )
		{
			time = 0.0f;
			debounce = 1;
		}

		tsMix( ctx );
	}

	tsFreeSound( &airlock );
	tsFreeSound( &rupee1 );
	tsFreeSound( &rupee2 );
}

int main( )
{
	int frequency = 44000; // a good standard frequency for playing commonly saved OGG + wav files
	int latency_in_Hz = 15; // a good latency, too high will cause artifacts, too low will create noticeable delays
	int buffered_seconds = 5; // number of seconds the buffer will hold in memory. want this long enough in case of frame-delays
	int use_playing_pool = 0; // non-zero uses high-level API, 0 uses low-level API
	int num_elements_in_playing_pool = use_playing_pool ? 5 : 0; // pooled memory array size for playing sounds

	// initializes direct sound and allocate necessary memory
	tsContext* ctx = tsMakeContext( GetConsoleWindow( ), frequency, latency_in_Hz, buffered_seconds, num_elements_in_playing_pool );


	if ( !use_playing_pool )
	{
		// play airlock sound upon startup and loop it (or play the ogg file,
		// change the if statement inside to try each out),
		// press space to play jump sound
		LowLevelAPI( ctx );
	}

	else
	{
		// Same as LowLevelApi, but uses the internal tsPlayingSound memory
		// pool. This can make it easier for users to generate tsPlayingSound
		// instances without doing manual memory management. Pressing space
		// once quickly will play a rupee noise (5 rupee pickup from ALTTP).
		// If space is held more rupees will keep coming :)
		HighLevelAPI( ctx );
	}


	// shuts down direct sound, frees all used memory
	tsShutdownContext( ctx );
}

// create implementation
#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

//
//  main.c
//  module
//
//  Created by Christopher Latina on 11/5/15.
//  Copyright © 2015 Christopher Latina. All rights reserved.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "portaudio.h"
#include "fft.h"
#include "spectral_features.h"



#import <CoreAudio/CoreAudio.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreServices/CoreServices.h>
#import <Carbon/Carbon.h>

/*
 ** Note that many of the older ISA sound cards on PCs do NOT support
 ** full duplex audio (simultaneous record and playback).
 ** And some only support full duplex at lower sample rates.
 */
#define SAMPLE_RATE         (44100)
#define PA_SAMPLE_TYPE      paFloat32
#define FRAMES_PER_BUFFER   (512)


typedef float SAMPLE;

static int onsetCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData );

FFT fft;
SpectralFeatures features;
float *spectrum;

static int gNumNoInputs = 0;
/* This routine will be called by the PortAudio engine when audio is needed.
 ** It may be called at interrupt level on some machines so don't do anything
 ** that could mess up the system like calling malloc() or free().
 */
static int onsetCallback( const void *inputBuffer, void *outputBuffer,
                         unsigned long framesPerBuffer,
                         const PaStreamCallbackTimeInfo* timeInfo,
                         PaStreamCallbackFlags statusFlags,
                         void *userData )
{
    SAMPLE *out = (SAMPLE*)outputBuffer;
    const SAMPLE *in = (const SAMPLE*)inputBuffer;
    unsigned int i;
    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) userData;
    const int signalSize = FRAMES_PER_BUFFER;
    float flux;
    float centroid;
    
    if( inputBuffer == NULL )
    {
        for( i=0; i<framesPerBuffer; i++ )
        {
            *out++ = 0;  /* left - silent */
            *out++ = 0;  /* right - silent */
        }
        gNumNoInputs += 1;
    }
    else
    {
        spectrum = getSpectrum(&fft, in, signalSize);
        
        extractSpectralFeatures(&features, spectrum, signalSize);
        flux = getSpectralFlux(&features);
        centroid = getSpectralCentroid(&features);
        
        for( i=0; i<framesPerBuffer; i++ )
        {
            *out++ = 0; //*in++;     /* left  - clean */
            *out++ = 0; //*in++;     /* right - clean */
        }
    }
    
    return paContinue;
}



/*******************************************************************/
int main(void);
int main(void)
{
    PaStreamParameters inputParameters, outputParameters;
    PaStream *stream;
    PaError err;
    
    err = Pa_Initialize();
    if( err != paNoError ) goto error;
    
    inputParameters.device = Pa_GetDefaultInputDevice(); /* default input device */
    if (inputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default input device.\n");
        goto error;
    }
    inputParameters.channelCount = 2;       /* stereo input */
    inputParameters.sampleFormat = PA_SAMPLE_TYPE;
    inputParameters.suggestedLatency = Pa_GetDeviceInfo( inputParameters.device )->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = NULL;
    
    outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
    if (outputParameters.device == paNoDevice) {
        fprintf(stderr,"Error: No default output device.\n");
        goto error;
    }
    outputParameters.channelCount = 2;       /* stereo output */
    outputParameters.sampleFormat = PA_SAMPLE_TYPE;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    
    /* Instantiate FFT */
    fft_new(&fft, FRAMES_PER_BUFFER);
    spectrum = malloc(FRAMES_PER_BUFFER * sizeof(double));
    
    /* Instantiate Spectral Features */
    spectralFeatures_new(&features, FRAMES_PER_BUFFER);
    
    err = Pa_OpenStream(
                        &stream,
                        &inputParameters,
                        &outputParameters,
                        SAMPLE_RATE,
                        FRAMES_PER_BUFFER,
                        0, /* paClipOff, */  /* we won't output out of range samples so don't bother clipping them */
                        onsetCallback,
                        NULL );
    if( err != paNoError ) goto error;
    
    err = Pa_StartStream( stream );
    if( err != paNoError ) goto error;
    
    printf("Hit ENTER to stop program.\n");
    getchar();
    err = Pa_CloseStream( stream );
    if( err != paNoError ) goto error;
    
    printf("Finished. gNumNoInputs = %d\n", gNumNoInputs );
    Pa_Terminate();
    return 0;
    
error:
    Pa_Terminate();
    fprintf( stderr, "An error occured while using the portaudio stream\n" );
    fprintf( stderr, "Error number: %d\n", err );
    fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
    return -1;
}


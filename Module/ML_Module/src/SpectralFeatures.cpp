//
//  SpectralFeatures.cpp
//  module
//
//  Created by Christopher Latina on 11/24/15.
//  Copyright © 2015 Christopher Latina. All rights reserved.
//

#include "SpectralFeatures.h"
#ifdef __arm__
    #include <wiringPi.h>
#endif

SpectralFeatures::SpectralFeatures (int numBins, int fs) {
    binSize = numBins;
    sampleRate = fs;
    prevFlux = 0.0;
    fifo = new float[binSize];
    fifo = initArray(fifo, binSize);
}

/*  Method to extract spectral features.
    Input arguments are the magnitude spectrum and block size */
void SpectralFeatures::extractFeatures(float* spectrum)
{
    float power = 0.0;
    float spectrum_sq[binSize];
    float spectrum_sum = 0.0;
    float spectrum_abs_sum = 0.0;
    
    for (int i=0; i<binSize; i++) {
        // Calculate the difference between the current block and the previous block's spectrum
        float diff = spectrum[i] - fifo[i];
        
        //Calculate the square
        power += diff * diff;
        
        //Sum of the magnitude spectrum
        spectrum_sum += spectrum[i];
        spectrum_abs_sum += fabsf(spectrum[i]);
        spectrum_sq[i] = spectrum[i] * spectrum[i];
    }
    
    /* Update fifo */
    setFifo(spectrum,binSize);
    
    /* Calculate Spectral Flux */
    calculateSpectralFlux(power);
    
    // Silent frames
    centroid = 0.0;
    crest = 0.0;
    
    if (spectrum_sum > 0.001){
        //Calculate Spectral Crest
        calculateSpectralCrest(spectrum, spectrum_abs_sum);
        
        //Calculate Spectral Centroid
        calculateSpectralCentroid(spectrum, spectrum_sum);
    }
}

void SpectralFeatures::calculateSpectralFlux(float power)
{
    //Calculate Spectral Flux
    flux = sqrt(power) / (binSize);
    
    // Low pass filter
    float alpha = 0.01;
    flux = (1-alpha)*flux + alpha * prevFlux;
    
    /* Save previous Spectral Flux */
    prevFlux = flux;
}

void SpectralFeatures::calculateSpectralCentroid(float* spectrum, float spectrum_sum)
{
    centroid = 0.0;
    for (int i=0; i<binSize; i++) {
        centroid += i*spectrum[i];
    }
    centroid = centroid / spectrum_sum;
    
    // Convert centroid from bin index to frequency
    centroid = (centroid / (float) binSize) * (sampleRate / 2);
    
    // TODO: This needs to be mapped to frequency and 1v / octave
}

void SpectralFeatures::calculateSpectralCrest(float* spectrum, float spectrum_abs_sum){
    crest = max_abs_array(spectrum, binSize) / spectrum_abs_sum;
}

void SpectralFeatures::calculateSpectralFlatness(float* spectrum) {
//    float min_thresh =1e-20;
    
//    float *xLog = log_array(spectrum, binSize);
//    xLog = add_array(spectrum,binsize);
//    
//    vtf     = exp(mean(XLog,1)) ./ (mean(X,1));
//    
//    // avoid NaN for silence frames
//    vtf (sum(X,1) == 0) = 0;
}

float SpectralFeatures::getSpectralFlux(){
    float thresh = 0.01;
    int onset = 0;
    /* Print if greater than threshold */
    if(flux > thresh){
        onset = 1;
        printf("Onset: %i, Flux: %f\n", onset, flux);
        printf("Centroid: %f, \n", centroid);
        #ifdef __arm__
            digitalWrite(8, 1);
        #endif
    }
    
    return flux;
}

float SpectralFeatures::getSpectralCrest(){
    return crest;
}

float SpectralFeatures::getSpectralFlatness(){
    return flatness;
}

float SpectralFeatures::getSpectralRolloff(){
    return rolloff;
}

float SpectralFeatures::getSpectralCentroid(){
//    printf("Centroid: %f, \n", centroid);
    return centroid;
    
}

float SpectralFeatures::getRMS(){
    return rms;
}
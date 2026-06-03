/*
  ==============================================================================

    FourierTransformation.h
    Created: 3 Jun 2026 10:24:23am
    Author:  Luke Amos

  ==============================================================================
*/

//This is the same code as a previous project built to explore how the FFT functions.

#pragma once
#include <JuceHeader.h>


class CircBuff{
    
public:
    
    void prepare(double sampleRateIn, int bufferSamples , int channelIn ){
        
        sampleRate = sampleRateIn;
        buffer.resize(bufferSamples, 0);
        writePosition = 0;
        channel = channelIn;
        
    }

    void processSample(juce::AudioBuffer<float> &inputBuffer){
        
        //Take the input samples and write over the oldest samples in the buffer
        
        auto samplesIn = inputBuffer.getReadPointer(channel);
        
        for(int i = 0; i < inputBuffer.getNumSamples() ; i++ ){
            
            buffer[(writePosition + i ) % buffer.size()] = samplesIn[i];

        }
        writePosition = (writePosition + inputBuffer.getNumSamples()) % buffer.size();
    }
    
    float orderedSamples(int positionInOrder){
        //getting the samples in order from oldest to newest no matter when in the circular buffer they are
        //The relationship of write position and oldestSample is writePos + 1 , so for an input of position we must return buffer [writepos +1 + position ]
        
        return buffer[(writePosition + 1 + positionInOrder)%buffer.size()];
    }
    
private:
    
    std::vector<float> buffer;
    int writePosition;
    double sampleRate;
    int channel;
    
};

struct Complex{
    
    float imaginary;
    float real;
};


//Apply windowing functions in the DFT and FFT to keep the CircBuff clean

class FFT{
    
public:
    
    void prepare(int FFTSize, float sampleRateIn){
        
        //Set up , including setting up the windowing function (Hann)
        sampleRate = sampleRateIn;
        fftSize = FFTSize;
        windowing.resize(fftSize, 0.0f);
        complexOut.resize(fftSize/2, {0.0f , 0.0f});
        orderedBuffer.resize(fftSize, 0.0f);
        
        //Setting up the Hann values for use on multiplying
        for(int i = 0 ; i < windowing.size() ; i ++){
            
            windowing[i] = 0.5 * (1 - std::cos((2 * M_PI) * i / (windowing.size()-1)));
            
        }
        
        //Only running a singlular left channel
        circBuffer.prepare(sampleRate, fftSize, 0);
    }
    
    
    void process(juce::AudioBuffer<float>& inputBuffer , bool halfBins = true){
        
        circBuffer.processSample(inputBuffer); // write the new block of audio to the circular buffer.
        
        //order the data from circular buffer and multiply by the window value
        for(int i = 0 ; i < orderedBuffer.size() ; i++){
            
            orderedBuffer[i] = circBuffer.orderedSamples(i) * windowing[i];
        }
        
        complexOut = calcFFT(orderedBuffer);
        
        if(halfBins){
            complexOut.resize(fftSize / 2); //Only want the first half of the fft the rest is unwanted data
        }
    }
    
    std::vector<Complex> calcFFT(std::vector<float> &input){
        
        //Base case if the size of the input is 1 then return itself as a complex number and triggers the butterfly calculations one caller level up
        //If the size is bigger than 1 then split and recurse until it reaches 1
        if(input.size() == 1 ){
            
            return { Complex{ input[0], 0.0f } };
        }
        
        std::vector<float> even;
        std::vector<float> odd;
        
        for(int i = 0 ; i < input.size() ; i ++ ){
            
            if(i % 2 == 0 ){ //is Even
                
                even.push_back(input[i]);
            }else {
                
                odd.push_back(input[i]);
            }
        }
        
        //Recursivly calling the function to fill it from the bottom up, the first call stops here until it reaches input.size() ==1 whereby it finally reaches a return value and begins the below functions inside the even and odd variables which build back up applying the butterfly computation as it goes
        std::vector<Complex> E = calcFFT(even);
        std::vector<Complex> O = calcFFT(odd);
        
        //Once both Even and Odd recursive functions are finished we combine their results on every level on the way back up , If looking from the top down the deepest level combines size-1 base cases into size-2 results until the top level produces the final full output.
        std::vector<Complex> result(input.size());
        
        for(int k = 0 ; k < input.size()/ 2 ;k++ ){
            
            float angle = -(2 * M_PI) * k/input.size(); //Calculate the angle, neg flips the sine component which is needed in a forward DFT
            
            Complex W = { .imaginary = std::sin(angle) , .real = std::cos(angle) }; //  W_k = e^{-i * 2pi * k/n}, this is then using eulers rule to calculate teh cos and sin parts
            
            Complex temp; // from the notes this is just W * O[k]
            temp.real = W.real * O[k].real - W.imaginary * O[k].imaginary; // when completing complex mult real = ac - bd , imaginary = ad + bc
            temp.imaginary =  W.real * O[k].imaginary + W.imaginary * O[k].real;
            //So then result becomes
            
            result[k] = {E[k].real + temp.real, E[k].imaginary + temp.imaginary};
            result[k + (input.size() / 2)] = {E[k].real - temp.real ,E[k].imaginary - temp.imaginary }; //Identical but signs flipped
        }
        
        return result;
    }

    float getMagnitude(int binIndex){
        
        if(binIndex < complexOut.size()){
            return std::sqrt( std::pow(complexOut[binIndex].real, 2) + std::pow(complexOut[binIndex].imaginary, 2));
        }else{
            return -0.0f;
        }
    }
    
    float getPhase(int binIndex){
        
        return std::atan2(complexOut[binIndex].imaginary, complexOut[binIndex].real);
    }
    
    float getMagnitudeDb(int binIndex){ //return the magnitude in dB , helpful for use in displaying the data in the frequency domain
        

        return 20.0f * std::log10( std::max(getMagnitude(binIndex), 0.0000001f) );
    }
    
    int getFFTSize(){
        return fftSize;
    }
    
    std::vector<Complex> getComplexOut(){
        
        return complexOut;
    }
    
private:
    
    CircBuff circBuffer;
    std::vector<float> windowing;
    std::vector<Complex> complexOut;
    std::vector<float> orderedBuffer;
    
    int fftSize;
    float sampleRate;
};


//Inverse Fourier Transformation built on previosu FFT function

class IFFT{
    
public:
    
    void prepare(int FFTSize, float sampleRateIn){
        
        //Set up , including
        fftSize = FFTSize;
        finalResultsOut.resize(fftSize, 0.0f);
        
    }
    
    
    void process(std::vector<Complex>& inputComplex){
        
        
        finalResultsOut = calcIFFT(inputComplex);
        
    }
    
    std::vector<Complex> calcIFFTHelper(std::vector<Complex> &input){
        
        //Base case if the size of the input is 1 then return itself as a complex number and triggers the butterfly calculations one caller level up
        //If the size is bigger than 1 then split and recurse until it reaches 1
        if(input.size() == 1 ){
            
            return { input[0]};
        }
        
        std::vector<Complex> even;
        std::vector<Complex> odd;
        
        for(int i = 0 ; i < input.size() ; i ++ ){
            
            if(i % 2 == 0 ){ //is Even
                
                even.push_back(input[i]);
            }else {
                
                odd.push_back(input[i]);
            }
        }
        
        //Recursivly calling the function to fill it from the bottom up, the first call stops here until it reaches input.size() ==1 whereby it finally reaches a return value and begins the below functions inside the even and odd variables which build back up applying the butterfly computation as it goes
        std::vector<Complex> E = calcIFFTHelper(even);
        std::vector<Complex> O = calcIFFTHelper(odd);
        
        //Once both Even and Odd recursive functions are finished we combine their results on every level on the way back up , If looking from the top down the deepest level combines size-1 base cases into size-2 results until the top level produces the final full output.
        std::vector<Complex> result(input.size());
        
        for(int k = 0 ; k < input.size()/ 2 ;k++ ){
            
            float angle = +(2 * M_PI) * k/input.size(); //Calculate the angle, neg flips the sine component which is needed in a forward DFT
            
            Complex W = { .imaginary = std::sin(angle) , .real = std::cos(angle) }; //  W_k = e^{-i * 2pi * k/n}, this is then using eulers rule to calculate teh cos and sin parts
            
            Complex temp; // from the notes this is just W * O[k]
            temp.real = W.real * O[k].real - W.imaginary * O[k].imaginary; // when completing complex mult real = ac - bd , imaginary = ad + bc
            temp.imaginary =  W.real * O[k].imaginary + W.imaginary * O[k].real;
            //So then result becomes
            
            result[k] = {E[k].real + temp.real, E[k].imaginary + temp.imaginary};
            result[k + (input.size() / 2)] = {E[k].real - temp.real ,E[k].imaginary - temp.imaginary }; //Identical but signs flipped
        }
        
        return result;
    }

    std::vector<float> calcIFFT(std::vector<Complex>& input){
        
        std::vector<float> results;
        auto complexAngleFlip = calcIFFTHelper(input);
        
        for(auto compRes : complexAngleFlip){
            
            results.push_back((compRes.real)/((float)complexAngleFlip.size()));
            
        }
        
        return results;
    }
    
    std::vector<float> getResults(){
        
        return finalResultsOut;
    }
    
private:
    
    std::vector<float> finalResultsOut;
    int fftSize;
    
};

/**
 * FractalNoise.h
 * v. 1.0.0
 *
 * Fractal noise sums together 1 or more octaves of Perlin noise together to form Fractional or Fractal Brownian motion.
 *
 * Copyright Chris Little 2012
 * Author: Chris Little
 */

#ifndef _FRACTALNOISE_H_
#define _FRACTALNOISE_H_

#include "Perlin.h"

class FractalNoise {
public:
	FractalNoise();
	~FractalNoise();

	// Returns a noise value, typically in the range -1 to 1, given a 3D sample position.
	float noise(float sample_x, float sample_y, float sample_z);

	// Set the number of octaves to sum. Has no effect if parameter 'o' is less than 1.
	void setOctaves(int o);
	// Sets the persistence of each octave. Negative values or values greater than 0.5 (especially greater than 1.0) may produce bizarre results.
	void setPersistence(float p);
	// Sets the lacunarity of each octave. Values less than 2.0 may produce odd/undesirable results.
	void setLacunarity(float l);

	void setBaseFrequency(float f);
	void setBaseAmplitude(float f);

private:
	Perlin* m_perlinSource;

	int m_octaves; // Number of octaves to sum together. Default is 10.
	float m_persistence; // Amplitude multiplier for each subsequent octave. Default is 0.5.
	float m_lacunarity; // Frequency multiplier for each subsequent octave. Default is 2.0.
	float m_baseFrequency; // Initial starting frequency. Default is 1.0.
	float m_baseAmplitude; // Initial starting amplitude. Default is 1.0.
};

#endif
#pragma once

#include <array>

// LEVEL CONFIG

#define PHASE_COUNT 10
#define VALUE_TYPES_COUNT 10

static inline constexpr std::array<int, PHASE_COUNT> spawnNumberPhasesMedium = {
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
	3,
	3,
};

static inline constexpr std::array<int, PHASE_COUNT> spawnNumberPhasesEasy = {
	5,
	1,
	1,
	1,
	1,
	2,
	2,
	2,
	2,
	2,
};
// config 2D array for probabilities of the different chip values based on index
// { 2, 4, 8, 16 ... 1024 }, where each index has a probability between 0 and 1 to spawn
// and there are 10 phases for each of the numbers up to 1024 as well. The total sum of a phase array should be 1.
static inline constexpr std::array<std::array<float, VALUE_TYPES_COUNT>, PHASE_COUNT> configPhaseMedium = {{
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.8, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.6, 0.2, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.2, 0.6, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.4, 0.4, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.1, 0.2, 0.3, 0.4, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.2, 0.4, 0.3, 0.1, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.1, 0.1, 0.3, 0.2, 0.2, 0.1, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.2, 0.3, 0.3, 0.1, 0.1, 0.0 }
}};

static inline constexpr std::array<std::array<float, VALUE_TYPES_COUNT>, PHASE_COUNT> configPhaseEasy = {{
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 }
}};

constexpr float constexpr_pow(float base, int exp) {
    float res = 1.0f;
    float abs_exp = (exp < 0) ? -exp : exp;
    for (int i = 0; i < abs_exp; ++i) res *= base;
    return (exp < 0) ? 1.0f / res : res;
}

constexpr float constexpr_round(float x) {
    // Simple version for positive numbers:
    // return static_cast<float>(static_cast<int>(x + 0.5f));
    
    // Robust version handling negatives:
    if (x < 0) {
        return static_cast<float>(static_cast<int>(x - 0.5f));
    }
    return static_cast<float>(static_cast<int>(x + 0.5f));
}

#define DISTRIBUTION_RESOLUTION 30

typedef std::array<int, DISTRIBUTION_RESOLUTION> ValueDistribution;
// generate the distribution of values based on the configuration of probabilities
static inline consteval std::array<ValueDistribution, PHASE_COUNT> generatePhaseDistributions(const std::array<std::array<float, VALUE_TYPES_COUNT>, PHASE_COUNT>& probabilities) {
	std::array<ValueDistribution, PHASE_COUNT> result{};

	// iterate throught the 2D array of probabilities
	// for each array of probabilities (phaseProbs)
	// iterate through each probability, and then
	// fill the corresponding phase array (each array entry in the probably array)
	// with the number of values corresponding to its probabibility
	for (int i = 0; i < PHASE_COUNT; ++i) {
		auto& phaseProbs = probabilities[i];
		auto& phaseResult = result[i];

		// keep track of the phase array index here
		// since the total of phase arrays probability should sum to 1
		int phaseValueIdx = 0;
		for (int j = 0; j < VALUE_TYPES_COUNT; ++j) {
			
			float currPhaseProb = phaseProbs[j];
			// skip over 0 probability values
			if (currPhaseProb == 0) continue;

			int entryValue = static_cast<int>(constexpr_pow(2, j+1));
			int entryLength = static_cast<int>(constexpr_round(currPhaseProb*DISTRIBUTION_RESOLUTION));
			
			// clamp in case some prob. is greater than 1.0
			if (entryLength > DISTRIBUTION_RESOLUTION) {
				entryLength = DISTRIBUTION_RESOLUTION;
			}
			// entryLength will be a fraction of the Distribution Resolution
			// iterate through the phase array, inserting the same entry value entryLength number of times
			for (int k = 0; k < entryLength && phaseValueIdx < DISTRIBUTION_RESOLUTION; ++k) {
				phaseResult[phaseValueIdx] = entryValue;

				phaseValueIdx++;
			}
			// phase array has been completely filled
			if (phaseValueIdx >= DISTRIBUTION_RESOLUTION) break;
		}	
	}

	return result;
}

static inline constexpr auto distributionMedium = generatePhaseDistributions(configPhaseMedium);
static inline constexpr auto distributionEasy = generatePhaseDistributions(configPhaseEasy);

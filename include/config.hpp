#pragma once

#include <array>
#include <cassert>

#define TARGET_FPS 120
// screen defaults
#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720
// centimeters/pixel at 1280x720px
#define SCREEN_UNIT 1.0f
#define SCREEN_RESIZE_RATE 300

// TODO: make constexpr
#define UNIT_RATIO (sqrtf(powf(static_cast<float>(SCREEN_WIDTH), 2.0f) + powf(static_cast<float>(SCREEN_HEIGHT), 2.0f)))

#define INTRO_TIME_MS 3000

#define RAYLIB_LOGO_SIZE 200

#define HEX_SIZE 80.0f
// Hex grid extent (half width/height) - 1 (center hex)
// or number of onion hex layers around center hex
#define GRID_EXTENT 2

#define CHIP_FONT_SIZE 52

#define PHASE_COUNT 10
#define VALUE_TYPES_COUNT 10
// config 2D array for probabilities of the different chip values based on index
// { 2, 4, 8, 16 ... 1024 }, where each index has a probability between 0 and 1 to spawn
// and there are 10 phases for each of the numbers up to 1024 as well. The total sum of a phase array should be 1.
static inline constexpr std::array<std::array<float, VALUE_TYPES_COUNT>, PHASE_COUNT> phaseValueDistributionConfig = {{
	{ 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.2, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.5, 0.5, 0.0, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0 }
}};

// TODO: move below stuff somewhere other than config
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
// generate the distribution of values based on the configuration of probabilities
static inline consteval std::array<std::array<int, DISTRIBUTION_RESOLUTION>, PHASE_COUNT> generatePhaseDistributions(const std::array<std::array<float, VALUE_TYPES_COUNT>, PHASE_COUNT>& probabilities) {
	std::array<std::array<int, DISTRIBUTION_RESOLUTION>, PHASE_COUNT> result{};

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

static inline constexpr auto randomizedPhaseMap = generatePhaseDistributions(phaseValueDistributionConfig);


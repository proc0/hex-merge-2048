#include "chip.hpp"
#include "config.hpp"
#include "tool.hpp"

#include "raylib.h"
#include "raymath.h"
#include "type.hpp"

void Chip::load(Vector2 position) {
	// setPosition(position);
	// setScale(1.0f);
	// setSize(HEX_SIZE);

	// setFontProps({ fontXOne*-0.5f, CHIP_FONT_SIZE*-0.5f }, CHIP_FONT_SIZE);
	// current[FONTSCALE] = 1.0f;	
	// source[FONTSCALE] = 1.0f;	
	// target[FONTSCALE] = 1.0f;

	// source[BORDERSIZE] = 1.0f;
	// current[BORDERSIZE] = 1.0f;
	// target[BORDERSIZE] = 1.0f;

	// setRotation(0.0f);
	// setColor(primaryColor);

	float fontXOne = MeasureText("2", CHIP_FONT_SIZE);

	// std::array<std::pair<int, float>, PROPS_SIZE> initP = {
	// 	std::make_pair(POSX, position.x),
	// 	{ POSY, position.y },
	// 	{ SCALE, 1.0f },
	// 	{ SIZE, HEX_SIZE },
	// 	{ FONTX, fontXOne*-0.5f },
	// 	{ FONTY, CHIP_FONT_SIZE*-0.5f },
	// 	{ FONTSIZE, CHIP_FONT_SIZE },
	// 	{ FONTSCALE, 1.0f },
	// 	{ BORDERSIZE, 1.0f },
	// 	{ ROT, 0.0f },
	// 	{ COLR, primaryColor.r },
	// 	{ COLG, primaryColor.g },
	// 	{ COLB, primaryColor.b },
	// 	{ COLA, primaryColor.a },
	// };
	StackMap<int, float, PROPS_SIZE> initProps = {
		{ POSX, position.x },
		{ POSY, position.y },
		{ SCALE, 1.0f },
		{ SIZE, HEX_SIZE },
		{ FONTX, fontXOne*-0.5f },
		{ FONTY, CHIP_FONT_SIZE*-0.5f },
		{ FONTSIZE, CHIP_FONT_SIZE },
		{ FONTSCALE, 1.0f },
		{ BORDERSIZE, 1.0f },
		{ ROT, 0.0f },
		{ COLR, primaryColor.r },
		{ COLG, primaryColor.g },
		{ COLB, primaryColor.b },
		{ COLA, primaryColor.a },
	};
	setProps({{ initProps.data.data(), initProps.size }}, true, true, true);
}

// void Chip::reload(Vector2 position, float scale, float size, float fontSize, float rotation, Color color) {
// 	setPosition(position);
// 	setScale(scale);
// 	setSize(size);
// 	setFontSize(size);
// 	setRotation(rotation);
// 	setColor(color);
// }

void Chip::reset() {
	enabled = false;
	absorbed = false;
	merged = false;

	for (auto& f : frame) {
		f = 0;
	}
	framePropsActive = 0;
	state = State::Chip::READY;
}

void Chip::sync() {
	// current[SCALE] = target[SCALE];
	// setFontSyncProps();
	// current[BORDERSIZE] = target[BORDERSIZE];

	// TODO: this could be a constexpr since it's always the same
	constexpr std::array<int, 6> syncMap = {
		SCALE,
		BORDERSIZE,
		SCALE,
		FONTX,
		FONTY,
		FONTSCALE,
	};
	applyPropChanges(syncMap);
	// applyPropChanges({{ syncMap.data.data(), syncMap.size }});

	// TraceLog(LOG_INFO, "CHIP: END MOVE SYNC");
	if (merged) {
		merged = false;
		enabled = false;
	}
	if (absorbed) {
		value = nextValue;
		absorbed = false;
	}

	// if (merged) {
	// 	merged = false;
	// 	enabled = false;
	// }
}

void Chip::render() const {
	float radius = current[SIZE]*current[SCALE];
	DrawPoly({ current[POSX], current[POSY] }, 6, radius, current[ROT], { static_cast<unsigned char>(current[COLR]), static_cast<unsigned char>(current[COLG]), static_cast<unsigned char>(current[COLB]), static_cast<unsigned char>(current[COLA]) });
	DrawPolyLinesEx({ current[POSX], current[POSY] }, 6, radius, current[ROT], current[BORDERSIZE], BLACK);
	
	const char* textValue = TextFormat("%d", value);
	// float fontWidth = MeasureText(textValue, current[FONTSIZE]);
	DrawText(textValue, current[POSX]+current[FONTX], current[POSY]+current[FONTY], current[FONTSIZE]*current[FONTSCALE], secondaryColor);
}

State::Chip Chip::update() {
	// bool stillMoving = state == State::Chip::MOVING;
	for (int i = 0; i < PROPS_SIZE; ++i) {
		int& currentFrame = frame[i];
		if (currentFrame > 0) {
			// float progress = ANIM_EASE_IN_QUAD[i];
			current[i] = Lerp(source[i], target[i], ANIM_EASE_IN_QUAD[currentFrame]);
			currentFrame++;
			if (currentFrame >= ANIM_EASE_IN_QUAD_MAX_IDX) {
				currentFrame = 0;
				framePropsActive--;
				// current[i] = target[i];
			}
		}
	}

	if (state == State::Chip::MOVING && framePropsActive <= 0) {
            // TraceLog(LOG_INFO, "DONE Animating moving chips.");
		// NOTE: this only gets called for MOVING chips
		// if non-moving chips need prop sync, use sync()
		// TODO: refactor this pattern into more consistent/obvious
		state = State::Chip::READY;
		// sync prop states
		// TODO: abstract this pattern
		// 1. set target props in merge or move
		// 2. current props get copied to source props
		// 3. (optional) source and target are lerped into current
		// 4. at some endpoint (i.e. right here) current is set to target
		
		// NOTE: this is for syncing the delay prop change for MOVE
		// maybe the restore value could be on source instead?
		// TODO: create a generic applyPropChanges, and perhaps one for restoring from target, and one from restoring from source
		// current[SCALE] = target[SCALE];
		// setFontSyncProps();
		// current[BORDERSIZE] = target[BORDERSIZE];
		// // TraceLog(LOG_INFO, "CHIP: END MOVE SYNC");
		// if (merged) {
		// 	merged = false;
		// 	enabled = false;
		// }
	}

	return state;
}

void Chip::place(Hex::Point hex, Vector2 position, int val) {
	setPosition(position);
	currentHex = hex;
	value = val;
	nextValue = val;
	enable();

	// update font properties 
	// float fontWidth = MeasureText(TextFormat("%d", value), current[FONTSIZE]);
	// setFontProps({ fontWidth*-0.5f, current[FONTSIZE]*-0.5f }, current[FONTSIZE], current[FONTSCALE]);
	updateFont(current[FONTSIZE]);


	StackMap<int, float, 2> scaledPropSources;
	scaledPropSources.insert(SCALE, 1.1f);
	scaledPropSources.insert(FONTSCALE, 1.1f);
	animatePropSources({{ scaledPropSources.data.data(), scaledPropSources.size }});

	// state = State::Chip::MOVING;

	// source[SCALE] = 1.12f;
	// current[SCALE] = 1.12f;
	// target[SCALE] = 1.0f;
	// frame[SCALE] = 1;

	// source[FONTSCALE] = 1.12f;
	// current[FONTSCALE] = 1.12f;
	// target[FONTSCALE] = 1.0f;
	// frame[FONTSCALE] = 1;

	// framePropsActive += 2;
}

// NOTES: there are two kinds of property changes
// 1. Property changes that animate
// These will set current to source, then set the new value on target
// Then the animation frame flags are set to one in frame array
// Then the global animation flag counter is incremented by how many props were set (framePropsActive)
// Along with setting the state to MOVING, World will also know when Chip is MOVING and load it on its own array to process
// It will keep calling update where the props will be Lerped until framePropsActive is 0, then it chagnes back to READY

// 2. Property changes that do not animate
// These just need to be delayed applied
// First you set current to source, then set the new value on target (technically source is not needed as there is no animation)
// Note that no animation flag is set, or state set to MOVING either, or framePropsActive incremented
// Then in a sync function, the properties update according to the configuration, the target is set to current etc.

// Summary: Two kinds of proprety changes, ones that need animated, ones that just need delayed
// There are two kinds of animations, going from source to target, and from target to source, 
// in the sense that one of them will have the changed property, and either the property will 
// animate into the target, i.e. source (unchanged) -> target (new value), or animate away from source,
// i.e. source (new value) -> target (unchanged)
// current value will store the intermediate Lerp value, or in the case of a delay, do nothing

void Chip::translate(Hex::Point hex, Vector2 position) {
	currentHex = hex;
	
	// source[POSX] = current[POSX];	
	// target[POSX] = position.x;
	
	// source[POSY] = current[POSY];	
	// target[POSY] = position.y;

	// state = State::Chip::MOVING;

	// frame[POSX] = 1;
	// frame[POSY] = 1;

	// framePropsActive += 2;

	StackMap<int, float, 2> moveTargets;
	moveTargets.insert(POSX, position.x);
	moveTargets.insert(POSY, position.y);

	animatePropTargets({{ moveTargets.data.data(), moveTargets.size }});

	// none-animated props
	// TODO: abstract this into a 
	// generic setTargetProps of some kind

	// source[SCALE] = 1.0f;
	// source[FONTSCALE] = 1.0f;
	// source[BORDERSIZE] = 1.0f;

	StackMap<int, float, 3> delayMoveSources;
	// delayMoveSources.insert(SCALE, 1.00f);
	// delayMoveSources.insert(FONTSCALE, 1.00f);
	// delayMoveSources.insert(BORDERSIZE, 1.00f);
	// delayPropChanges({{ delayMoveSources.data.data(), delayMoveSources.size }});
	delayMoveSources.insert(SCALE, 1.08f);
	delayMoveSources.insert(FONTSCALE, 1.08f);
	delayMoveSources.insert(BORDERSIZE, 1.08f);
	// delayPropRestore({{ delayMoveSources.data.data(), delayMoveSources.size }});
	// TODO: Why does setProps not work the same way as delayPropRestore
	setProps({{ delayMoveSources.data.data(), delayMoveSources.size }}, false, true, false);


	/// backup
	// current[SCALE] = 1.08f;
	// target[SCALE] = 1.0f;

	// current[FONTSCALE] = 1.08f;
	// target[FONTSCALE] = 1.0f;
	
	// current[BORDERSIZE] = 2.5f;
	// target[BORDERSIZE] = 1.0f;
}

int Chip::merge(Chip& other) {
	nextValue += other.value;
	other.translate(currentHex, getTargetPosition());
	other.merged = true;
	absorbed = true;

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", nextValue), current[FONTSIZE]);
	// setFontTargetProps({ fontWidth*-0.5f, current[FONTSIZE]*-0.5f }, current[FONTSIZE], current[FONTSCALE]);
	StackMap<int, float, 2> fontPosTargets;
	fontPosTargets.insert(FONTX, fontWidth*-0.5f);
	fontPosTargets.insert(FONTY, current[FONTSIZE]*-0.5f);
	delayPropChanges({{ fontPosTargets.data.data(), fontPosTargets.size }});

	return nextValue;
}

void Chip::updateFont(float fontSize) {

	// update font properties 
	float fontWidth = MeasureText(TextFormat("%d", value), fontSize);
	float fontX = fontWidth*-0.5f;
	float fontY = fontSize*-0.5f;

	current[FONTX] = fontX;	
	source[FONTX] = fontX;	
	target[FONTX] = fontX;

	current[FONTY] = fontY;	
	source[FONTY] = fontY;	
	target[FONTY] = fontY;

	current[FONTSIZE] = fontSize;	
	source[FONTSIZE] = fontSize;	
	target[FONTSIZE] = fontSize;
}

// void Chip::applyPropChanges(FlatMapView<int, float> propMap) {

// 	auto it = propMap.data.begin();
// 	while(it != propMap.data.end()) {
// 		int key = it->first;
// 		// float sourceVal = it->second;

// 		current[key] = target[key];

// 		it++;
// 	}
// }

void Chip::applyPropChanges(const std::span<const int> syncKeys) {

	auto it = syncKeys.begin();
	while(it != syncKeys.end()) {
		int key = *it;

		current[key] = target[key];

		it++;
	}
}

// TODO: replace all the other getters and setters with setProps if possible
void Chip::setProps(FlatMapView<int, float> propMap, bool setSource, bool setCurrent, bool setTarget) {

	auto it = propMap.data.begin();
	while(it != propMap.data.end()) {
		int key = it->first;
		float val = it->second;

		if (setSource) {
			source[key] = val;
		}
		
		if (setCurrent) {
			current[key] = val;
		}

		if (setTarget) {
			target[key] = val;
		}

		it++;
	}
}

void Chip::delayPropChanges(FlatMapView<int, float> targetMap) {

	auto it = targetMap.data.begin();
	while(it != targetMap.data.end()) {
		int key = it->first;
		float targetVal = it->second;
		
		source[key] = current[key];
		target[key] = targetVal;

		it++;
	}
}

void Chip::animatePropSources(FlatMapView<int, float> sourceMap) {

	auto it = sourceMap.data.begin();
	while(it != sourceMap.data.end()) {
		int key = it->first;
		float sourceVal = it->second;
		
		source[key] = sourceVal;
		target[key] = current[key];
		frame[key] = 1;

		it++;
		framePropsActive++;
	}

	state = State::Chip::MOVING;
}

void Chip::animatePropTargets(FlatMapView<int, float> targetMap) {

	auto it = targetMap.data.begin();
	while(it != targetMap.data.end()) {
		int key = it->first;
		float targetVal = it->second;
		
		source[key] = current[key];
		target[key] = targetVal;
		frame[key] = 1;

		it++;
		framePropsActive++;
	}

	state = State::Chip::MOVING;
}

bool Chip::hasAbsorbed() const {
	return absorbed;
}

Hex::Point Chip::getCurrentHex() const {
	return currentHex;
}

Vector2 Chip::getPosition() const {
	return { current[POSX], current[POSY] };
}

Vector2 Chip::getTargetPosition() const {
	return { target[POSX], target[POSY] };
}

float Chip::getFontSize() const {
	return current[FONTSIZE];
}

int Chip::getId() const {
	return id;
}

int Chip::getValue() const {
	return value;
}

void Chip::setCurrentHex(Hex::Point hex) {
	currentHex = hex;
}

void Chip::setValue(int val) {
	value = val;
}

void Chip::addValue(int val) {
	value += val;
}

void Chip::setPosition(Vector2 position) {
	current[POSX] = position.x;	
	source[POSX] = position.x;	
	target[POSX] = position.x;

	current[POSY] = position.y;	
	source[POSY] = position.y;	
	target[POSY] = position.y;
}

// void Chip::setScale(float scale) {
// 	current[SCALE] = scale;	
// 	source[SCALE] = scale;	
// 	target[SCALE] = scale;
// }

void Chip::setSize(float size) {
	current[SIZE] = size;	
	source[SIZE] = size;	
	target[SIZE] = size;
}

// void Chip::setFontSize(float fontSize) {
// 	current[FONTSIZE] = fontSize;	
// 	source[FONTSIZE] = fontSize;	
// 	target[FONTSIZE] = fontSize;
// }

// void Chip::setFontProps(Vector2 position, float fontSize) {
// 	current[FONTX] = position.x;	
// 	source[FONTX] = position.x;	
// 	target[FONTX] = position.x;

// 	current[FONTY] = position.y;	
// 	source[FONTY] = position.y;	
// 	target[FONTY] = position.y;

// 	current[FONTSIZE] = fontSize;	
// 	source[FONTSIZE] = fontSize;	
// 	target[FONTSIZE] = fontSize;

// 	// current[FONTSCALE] = fontScale;	
// 	// source[FONTSCALE] = fontScale;	
// 	// target[FONTSCALE] = fontScale;
// }

// NOTE: this syncs delayed prop changes
// but could be used to sync animations (animations animate to their destination, in theory)
// It also syncs delay on SOURCE prop changes
// TODO: have one that syncs TARGTE prop changes? i.e. current[PROP] = source[PROP] ...
// would this be needed?
// void Chip::setFontSyncProps() {
// 	current[FONTX] = target[FONTX];	
// 	current[FONTY] = target[FONTY];	
// 	// current[FONTSIZE] = target[FONTSIZE];	
// 	current[FONTSCALE] = target[FONTSCALE];	
// }

// void Chip::setFontTargetProps(Vector2 position, float fontSize, float fontScale) {
// 	source[FONTX] = current[FONTX];	
// 	target[FONTX] = position.x;

// 	source[FONTY] = current[FONTY];	
// 	target[FONTY] = position.y;

// 	source[FONTSIZE] = current[FONTSIZE];	
// 	target[FONTSIZE] = fontSize;

// 	source[FONTSCALE] = current[FONTSCALE];	
// 	target[FONTSCALE] = fontScale;
// }

// void Chip::setRotation(float rotation) {
// 	current[ROT] = rotation;	
// 	source[ROT] = rotation;	
// 	target[ROT] = rotation;
// }

// void Chip::setColor(Color color) {
// 	current[COLR] = color.r;	
// 	source[COLR] = color.r;	
// 	target[COLR] = color.r;

// 	current[COLG] = color.g;	
// 	source[COLG] = color.g;	
// 	target[COLG] = color.g;

// 	current[COLB] = color.b;	
// 	source[COLB] = color.b;	
// 	target[COLB] = color.b;

// 	current[COLA] = color.a;	
// 	source[COLA] = color.a;	
// 	target[COLA] = color.a;
// }

void Chip::enable() {
	enabled = true;
}

void Chip::disable() {
	enabled = false;
}

bool Chip::active() const {
	return enabled;
}

bool Chip::available() const {
	return !enabled;
}

void Chip::unload() {

}
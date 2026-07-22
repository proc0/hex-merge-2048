#include "app.hpp"

int main(void){
    App* app = new App();
    
    app->load();
    app->start();
    app->unload(0, 0, app);

    delete app;

    return 0;
}

// MODE: time based mode "survival mode"
// new chips spawn at a time interval
// time interval accelerates as a factor of both maxValue and time
// goes on beyond 2048?

// MODE: classic mode "turn-based"
// chips spawn on every move that has a merge

// MODE: decaying/hardened mode 
// chips slowly harden so you can't merge them anymore
// higher number chips harden slowly

// MODE: LAB: music/rythm based mode
// chips spawn on the beat?
// moving is done on the beat?
// could be combined with time based?

// DONE: move gridlock logic to somewhere that does not depend on move inputs
// right now the game over screen only shows up after pressing a key
// DONE: scale chip numbers with screen
// DONE: scale chip size when moving (get bigger 1.1f scale)
// DONE: scale spawning chips, i.e. like they are landing on the board

// LAB: set a render target for chip backgrounds, only render text and effects every frame
// cache the chip backgrounds for every number

// TODO: add basic game, with an easy way to balance that can be calculated at compile time
// some kind of config that initializes the game, and potentially sets it up for other modes
// i.e. number of chips spawning; fine tuning the propabilites of which chips spawn;
// might require to think about phases to use phases as a way to configure these settings

// TODO: make animations frame independent
// TODO: add a way for different kinds of animation functions per prop, i.e. some ease in others ease out, etc

// TODO: add phases, changing color as maxValue increases
// change board and bg color
// grayscale light -> cool pastels -> warm pastels -> cool saturated -> light grayscale high contrast -> hot saturated -> dark grayscale high contrast

// LAB: add particle effects on background for phase effect
// particles get faster and faster as phase changes 
// background goes from cool colors to warm colors

// LAB: "RPG" mode, where different types of hexes show up
// some hexes explode and clear, others crystalize and obstruct
// hexes would have a few different types and show up randomly
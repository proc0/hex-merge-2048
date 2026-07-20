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

// TODO: move gridlock logic to somewhere that does not depend on move inputs
// right now the game over screen only shows up after pressing a key

// TODO: scale chip numbers with screen
// TODO: scale chip size when moving (get bigger 1.1f scale)
// LAB: set a render target for chip backgrounds, only render text and effects every frame
// cache the chip backgrounds for every number

// TODO: scale spawning chips, i.e. like they are landing on the board

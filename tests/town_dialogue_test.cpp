#include "../shared/town_dialogue.hpp"
#include <cassert>
int main() {
    using namespace town_dialogue;const world_regions::Position p{world_regions::Id::Clubhouse,12,10};
    Candidates c{};c[0]={town_npcs::baseId,100,{world_regions::Id::Clubhouse,12,9},true};
    assert(eligible(p,c[0]) && nearest(p,c)==&c[0]);
    auto bad=c[0];bad.id++;assert(!eligible(p,bad));bad=c[0];bad.type=1;assert(!eligible(p,bad));
    bad=c[0];bad.present=false;assert(!eligible(p,bad));
    assert(!eligible({world_regions::Id::SunnyPeak,12,10},c[0]));
    bad=c[0];bad.position={world_regions::Id::Clubhouse,14,9};assert(!eligible(p,bad));
    bad=c[0];bad.position={world_regions::Id::Clubhouse,15,10};assert(!eligible({world_regions::Id::Clubhouse,14,10},bad));
    c[1]={town_npcs::baseId+1,101,{world_regions::Id::Clubhouse,12,11},true};
    assert(nearest(p,c)->id==town_npcs::baseId); // Stable lower-ID tie, independent of registry iteration.
    State state;assert(state.open(p,c) && state.page==0);state.next();assert(state.page==1);state.next();assert(!state.visible());
    assert(state.open(p,c));c[0].position={world_regions::Id::Clubhouse,13,9};state.reconcile(p,c,true);assert(state.visible());
    c[0].present=false;state.reconcile(p,c,true);assert(!state.visible());
    assert(state.open(p,c));state.reconcile(p,c,false);assert(!state.visible());
}

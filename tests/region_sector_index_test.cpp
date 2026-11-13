#include "../TP_2023192003/TP_2023192003/region_sector_index.hpp"
#include <atomic>
#include <cassert>
#include <climits>
#include <set>
#include <thread>
int main() {
    using world_regions::Id;
    region_sectors::Index index;
    assert(index.put(1,{Id::Clubhouse,10,10}) && index.put(2,{Id::SunnyPeak,10,10}));
    assert(index.bucketCount()==2 && index.nearby({Id::Clubhouse,10,10},0)==std::unordered_set<int>{1});
    assert(index.nearby({Id::SunnyPeak,10,10},INT_MAX)==std::unordered_set<int>{2});
    assert(index.put(1,{Id::SunnyPeak,10,10}));
    assert(index.bucketCount()==1 && index.snapshot().size()==2 && index.memberships(1)==1);
    for(int i=0;i<50;++i) assert(index.put(1,{Id::SunnyPeak,10,10}));
    assert(!index.put(1,{Id::Invalid,0,0}) && !index.put(1,{Id::SunnyPeak,20,10}) && !index.put(-1,{Id::Clubhouse,10,10}));
    assert(index.memberships(1)==1 && index.nearby({},100).empty() && index.nearby({Id::Clubhouse,0,0},-1).empty());
    assert(index.remove(1) && !index.remove(1) && index.remove(2) && index.bucketCount()==0);
    assert(index.put(3,{Id::Clubhouse,19,16}) && index.put(4,{Id::Clubhouse,20,16}));
    assert(index.nearby({Id::Clubhouse,19,16},1).size()==2); // Adjacent local sectors.
    assert(index.nearby({Id::Clubhouse,19,16},0)==std::unordered_set<int>{3});
    assert(index.put(3,{Id::Clubhouse,20,16}) && index.bucketCount()==1 && index.memberships(3)==1);
    index.remove(3); index.remove(4);
    bool rejected=false; try { region_sectors::Index bad(0); } catch(const std::invalid_argument&) { rejected=true; } assert(rejected);
    std::atomic<bool> done{false};
    std::thread reader([&] {
        while(!done.load()) {
            std::set<int> ids;
            for(const auto& entry : index.snapshot()) {
                assert(ids.insert(entry.id).second);
                assert(entry.key.region==Id::Clubhouse || entry.key.region==Id::SunnyPeak);
                assert(entry.key.x>=0 && entry.key.x<=1 && entry.key.y==0);
            }
            assert(index.nearby({Id::Clubhouse,19,16},7).size()<=5);
        }
    });
    std::vector<std::thread> writers;
    for(int writer=0;writer<4;++writer) writers.emplace_back([&,writer] {
        for(int i=0;i<3000;++i) {
            const world_regions::Position p=i%2 ? world_regions::Position{Id::Clubhouse,21,16} : world_regions::Position{Id::SunnyPeak,10,10};
            assert(index.put(writer,p) && index.put(99,p)); // Competing updates to one ID must not duplicate membership.
            if(i%3==0) index.remove(writer);
        }
    });
    for(auto& writer : writers) writer.join(); done=true; reader.join();
    for(int id : {0,1,2,3,99}) index.remove(id);
    assert(index.snapshot().empty() && index.bucketCount()==0);
}

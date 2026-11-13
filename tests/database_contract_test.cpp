#include "../TP_2023192003/TP_2023192003/database_contract.hpp"
#include "../TP_2023192003/TP_2023192003/game_database.hpp"
#include <cassert>
#include <vector>
using namespace database_contract;
struct Reader {
    bool metadata=true; int cursor=0,failAt=-1;
    std::array<Column,count> columns;
    std::vector<Values> rows;
    Lengths lengths;
    Reader() {
        for(std::size_t i=0;i<count;++i) columns[i]={column(i),true};
        lengths.fill(4); rows.push_back({123,42,10,10,1,0,5,20,1});
    }
    bool describe(std::array<Column,count>& output) { output=columns; return metadata; }
    Fetch fetch(Values& output,Lengths& indicators) {
        if(cursor==failAt) return Fetch::Error;
        if(cursor>=static_cast<int>(rows.size())) return Fetch::End;
        output=rows[cursor++]; indicators=lengths; return Fetch::Row;
    }
};
static void rejected(Reader reader,Result expected=Result::Error) {
    Values sentinel; sentinel.fill(777); auto output=sentinel;
    assert(readSingle(reader,123,output)==expected && output==sentinel);
    int creates=0;
    const auto lookup=expected==Result::NotFound ? LoginLookupResult::NotFound : LoginLookupResult::Error;
    if(expected==Result::Error) {
        assert(load_or_create_user([&]{return lookup;},[&]{++creates; return true;})==LoginLookupResult::Error);
        assert(creates==0);
    }
}
int main() {
    Reader good; Values output{}; assert(readSingle(good,123,output)==Result::Found && output[0]==123 && output[1]==42);
    Reader missing; missing.rows.clear(); rejected(missing,Result::NotFound);
    Reader metadata; metadata.metadata=false; rejected(metadata);
    for(std::size_t i=0;i<count;++i) {
        Reader wrongName; wrongName.columns[i].name="OTHER"; rejected(wrongName);
        Reader wrongType; wrongType.columns[i].int32=false; rejected(wrongType);
        for(auto size : {-1ll,-4ll,0ll,3ll,8ll}) { Reader invalid; invalid.lengths[i]=size; rejected(invalid); }
    }
    Reader swapped; std::swap(swapped.columns[1],swapped.columns[2]); rejected(swapped);
    Reader wrongId; wrongId.rows[0][0]=999; rejected(wrongId);
    Reader duplicate; duplicate.rows.push_back(duplicate.rows[0]); rejected(duplicate);
    for(int fail : {0,1}) { Reader failed; failed.failAt=fail; rejected(failed); }
    for(int field=3;field<9;++field) for(int value : {-1,SHRT_MAX+1}) { Reader invalid; invalid.rows[0][field]=value; rejected(invalid); }
    Reader zeroHpLimit; zeroHpLimit.rows[0][7]=0; rejected(zeroHpLimit);
    Reader impossibleHp; impossibleHp.rows[0][3]=21; rejected(impossibleHp);
    assert(matchesColumn(0,"user_id",true) && !matchesColumn(9,"",true));
    Reader negative; negative.rows.clear(); Values sentinel{}; sentinel.fill(777); auto unchanged=sentinel;
    assert(readSingle(negative,-1,unchanged)==Result::Error && unchanged==sentinel);
}

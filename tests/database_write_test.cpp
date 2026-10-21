#include "../TP_2023192003/TP_2023192003/database_write.hpp"
#include <cassert>
#include <vector>

struct MockStatement {
    int failStep = -1, steps = 0, rows = 1;
    bool executed = false;
    std::vector<int> bound;
    bool next() { return steps++ != failStep; }
    bool prepare(const char*) { return next(); }
    bool bind(std::size_t index, std::int32_t& value) {
        assert(index == bound.size()+1);
        bound.push_back(value);
        return next();
    }
    bool execute() { executed = true; return next(); }
    bool affectedOneRow() { return next() && rows == 1; }
};

int main() {
    auto values = database_write::userValues(123,10,20,30,40,50,60,70,80);
    assert((values == std::array<std::int32_t,9>{{10,20,30,40,50,60,70,80,123}}));
    MockStatement success;
    assert(database_write::execute(success,"update",values));
    assert(success.steps == 12 && success.executed);
    assert((success.bound == std::vector<int>{10,20,30,40,50,60,70,80,123}));
    // Prepare, every individual bind, execute, and row-count retrieval failures.
    for (int fail=0;fail<12;++fail) {
        MockStatement mock;
        mock.failStep = fail;
        assert(!database_write::execute(mock,"update",values));
        assert(mock.steps == fail+1);
        assert(mock.executed == (fail >= 10));
    }
    for (int rows : {-1,0,2}) {
        MockStatement mock;
        mock.rows = rows;
        assert(!database_write::execute(mock,"update",values));
    }
}

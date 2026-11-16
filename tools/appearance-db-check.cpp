// Reuse the bounded, secret-safe connection/schema diagnostic harness.
#define wmain appearance_admin_not_executed
#include "appearance-db-admin.cpp"
#undef wmain

int wmain(int argc,wchar_t** argv) {
    if(argc!=3) return 2;
    const bool saveA=std::wcscmp(argv[2],L"SaveA")==0,saveB=std::wcscmp(argv[2],L"SaveB")==0;
    const bool verifyA=std::wcscmp(argv[2],L"VerifyA")==0,verifyB=std::wcscmp(argv[2],L"VerifyB")==0;
    if(!saveA && !saveB && !verifyA && !verifyB) return 2;
    Connection connection;
    if(!connection.open(argv[1],saveA || saveB)) return 1;
    int accounts=0;
    // Fixed previously-approved dedicated account. Never create an account.
    if(!scalar(connection.db,L"SELECT COUNT(*) FROM SERVER_TABLE1 WHERE USER_ID=900001299",accounts) || accounts!=1) {
        std::cerr<<"Dedicated existing account not uniquely present; stopped without writes.\n"; return 1;
    }
    std::mutex mutex; appearance_storage::OdbcStore store(connection.db,mutex);
    if(!store.schemaReadyUnlocked()) return 1;
    constexpr int account=900001299;
    const avatar_appearance::Value a{1,4,2,2},b{0,1,1,1};
    const auto expected=(saveA || verifyA) ? a : b;
    const auto before=store.read(account);
    if(saveA || saveB) {
        if(before.status!=appearance_storage::Status::Missing && (before.status!=appearance_storage::Status::Found
            || (before.value!=a && before.value!=b))) {
            std::cerr<<"Unexpected existing appearance; refused overwrite.\n"; return 1;
        }
        if(store.save(account,expected)!=appearance_storage::Status::Saved
            || store.save(account,{255,0,0,0})!=appearance_storage::Status::Error) {
            std::cerr<<"Appearance save/invalid-input check failed.\n"; return 1;
        }
    }
    const auto after=store.read(account);
    if(after.status!=appearance_storage::Status::Found || after.value!=expected) {
        std::cerr<<"Appearance restore mismatch.\n"; return 1;
    }
    std::cout<<"Appearance "<<(saveA || saveB ? "save/update + rejected invalid input" : "fresh-process restore")
        <<" PASS; approved existing test account only, no account creation/deletion.\n";
    return 0;
}

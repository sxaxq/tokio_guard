# tokio_guard

Simple DDoS guard

### Usage:
```
./tokio_guard.exe <guard_address> <wait_page.html> <banned_ip_db_name>
```
### Build g++:
```
g++ -lsqlite3 sqlitebebra.hpp sqlitebebra.cc tokioguard.cc -o tokioguard.exe
```

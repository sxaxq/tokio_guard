# tokio_guard

<hr>

#### About:
This ddos defender is able to enter suspicious IPs into the database and further prohibit them from entering the site.

#### Requirements:
boost.Asio, sqlite api, c++ 17

#### Usage:
```
./tokio_guard.exe <guard_address> <wait_page.html> <banned_ip_db_name>
```
#### Build g++:
```
g++ -lsqlite3 sqlitebebra.hpp sqlitebebra.cc tokioguard.cc -o tokioguard.exe
```

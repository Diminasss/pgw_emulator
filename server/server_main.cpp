#include "include/json_loader.h"
#include <iostream>

using std::cout;
using std::endl;


int main() {
    json_loader jsonLoader;
    jsonLoader.load("config/server_config.json");
    cout << jsonLoader.http_port;
    return 0;
}

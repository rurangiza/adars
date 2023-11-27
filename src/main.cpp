// The brain of the HTTP server

#include "../includes/Server.hpp"
#include "../includes/Client.hpp"
#include "../includes/Http.hpp"
#include "../includes/Cluster.hpp"
#include "../includes/Webserv.hpp"

int main(int ac, char **av)
{
    Cluster cluster;
    Data data;

    if (ac == 1) {
        try {
            data.config.parsing_file("conf/default.config", data);
        }
        catch (std::exception &e) {
            exit(Log::out(e.what()));
        }
    }
    else if (ac == 2) {
        try {
            data.config.parsing_file(av[1], data);
        }
        catch (std::exception &e) {
            exit(Log::out(e.what()));
        }
    }
    else {
        Log::error("Wrong numers of arguments");
        return (EXIT_FAILURE);
    }
    if (data.error.length())
        exit(Log::out(data.error));
        
    Log::success("CONFIGURATION FILE PARSING DONE");

    try {
        cluster.init(data.server_list);
        Log::success("CLUSTER INITIALIZED");
    }
    catch (std::exception& e) {
        exit(Log::out(e.what()));
    }
    cluster.watch();

    return EXIT_SUCCESS;
}

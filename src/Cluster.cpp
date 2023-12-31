#include "../includes/Cluster.hpp"

typedef vector< map<string, map<string, vector<string> > > > LocationsList;
typedef map< string, map<string, vector<string> > > LocationsMap;

Cluster::Cluster() : _size(0) {
    // if no activity after 3 minutes, 
    _timeout.tv_sec = 3 * 60;
    _timeout.tv_usec = 0;
}
Cluster::~Cluster()
{
    for (IteratorS it = this->begin(); it != this->end(); it++) {
        close((*it).second._socket);
    }
}

std::string get_host(std::vector<std::string> v)
{
    std::string s = v[0];
    s = s.substr(0, s.find_first_of(":"));
    return s;
}

void
Cluster::init(Serv_list servers)
{
    _size = servers.size();
    string tmp;

    Log::success("Configuration file parsing done"); 
    for(Serv_list::iterator it = servers.begin(); it != servers.end(); ++it)
    {

        Server s;
        s.set_location((*it).locations);

        for (map_vector_it server_data_it = it->server_data.begin(); server_data_it != it->server_data.end(); ++server_data_it)
        {
            for (vector<string>::iterator value_it = server_data_it->second.begin(); value_it != server_data_it->second.end(); value_it++)
            {
                if (server_data_it->first == "listen")
                    s.set_port(server_data_it->second[0]);
                else if (server_data_it->first == "client_max_body_size")
                    s.set_request_body_size_limit(server_data_it->second[0]);
                else if (server_data_it->first == "root")
                    s.set_root(server_data_it->second[0]);
                else if (server_data_it->first == "host")
                    s.set_host(server_data_it->second[0]);
                else if (server_data_it->first == "index")
                    s.set_default_index(server_data_it->second[0]);
                else if (server_data_it->first == "server_name")
                    s.set_server_name(server_data_it->second[0]);
                else if (server_data_it->first == "error_page") {
                    s.set_default_error_page(server_data_it->second);
                }
           }
        }
        
        s.setup(IPV4,TCP,DEFAULT, BACKLOG);
        _servers[s._socket] = s;
    }
    _begin = _servers.begin();
    _end = _servers.end();

    std::cout << "Press CONTROL-C to quit\n";
}

void
Cluster::watch()
{
    int readable_fds, curr_fd, max_fd = 0, client_fd;

    // initialize data structure to store all known sockets (server and client)
    fd_set master_set, working_set;
    FD_ZERO(&master_set);
    for (IteratorS it = this->begin(); it != this->end(); it++) {
        FD_SET((*it).first, &master_set);
        max_fd = (*it).first;
    }

    do
    {
        working_set = master_set;
        // wait for something to read
        readable_fds = select(max_fd+1, &working_set, WRITE_FDS, ERROR_FDS, &this->_timeout);
        if (check::failed(readable_fds))
            break ;

        // identify which socket is readable
        for (curr_fd = 0; curr_fd <= max_fd && readable_fds ; curr_fd++) {
            if (FD_ISSET(curr_fd, &working_set)) {

                readable_fds -= 1;

                // server = new TCP connection
                if (this->find(curr_fd)) {
                    try { /* accept() */
                        client_fd = http::accept_connection(curr_fd);
                        FD_SET(client_fd, &master_set);
                        if (client_fd > max_fd)
                            max_fd = client_fd;
                    } catch (std::exception& e) {}
                }
                // client = new HTTP request
                else {
                    try { /* recv() & send() */
                        http::handle_request(curr_fd, *this);
                    } catch(std::exception& e) {
                        close(curr_fd);
                    }
                    FD_CLR(curr_fd, &master_set);
                }
            }
        }
    }
    while (isRunning);

    // clean up all opened sockets
    for (curr_fd = 0; curr_fd <= max_fd; ++curr_fd) {
        if (FD_ISSET(curr_fd, &master_set))
            close(curr_fd);
    }
}

std::map<int, Server>::iterator
Cluster::begin() { return _begin; }

std::map<int, Server>::iterator
Cluster::end() { return _end; }

int Cluster::size() { return _size; }

bool Cluster::find(int curr_fd)
{
    if (_servers.find(curr_fd) != _servers.end())
        return true;
        // return (*(_servers.find(curr_fd))).first;
    return false;
}

Server Cluster::getServerByPort(int port)
{
    // IteratorS it = this->begin();
    Server s;
    for (IteratorS it = this->begin(); it != this->end(); it++) {
        if ((*it).second.get_port() == port)
            s = (*it).second;
    }
    return s;
}

#ifndef UTILS_HPP
    #define UTILS_HPP

    #include <sstream>
    #include <string>

    #define CBLUE    "\033[0;94m"
    #define CYELLOW   "\033[0;33m"
    #define CGREEN   "\033[0;32m"
    #define CMAGENTA  "\033[0;35m"
    #define CRED      "\033[0;31m"
    #define CRESET    "\033[0m"

    class Utils {
        public:
        static std::string to_str(int num);
    };

#endif
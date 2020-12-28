#include <iostream>

#include "cxxopts.hpp"
#include "klijent.h"
#include "server.h"

int main(int argc, char **argv) {
    cxxopts::Options options(argv[0], "Key value store");

    options.add_options()("s,server", "Start server",
                          cxxopts::value<bool>()->default_value("false"))
        //
        ("c,cleint", "Start a client",
         cxxopts::value<bool>()->default_value("true"))
        //
        ("i,interactive", "Interactive option for client")
        //
        ("ip", "IP address", cxxopts::value<std::string>())
        //
        ("p,port", "Port number", cxxopts::value<std::string>())
        //
        ("com", "Command", cxxopts::value<std::vector<std::string>>())
        //
        ("con", "Listen backlog", cxxopts::value<std::string>())
        //
        ("h,help", "Print usage")
        //
        ;

    auto result = options.parse(argc, argv);

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 1;
    }
    if (result.count("s")) {
        options.parse_positional({"p", "con"});
        auto sresult = options.parse(argc, argv);
        if (!sresult.count("p") || !sresult.count("con"))
            std::cout << "Navedite port i broj konekcija" << std::endl;

        uint16_t p = std::stoi(sresult["p"].as<std::string>());
        int c = std::stoi(sresult["con"].as<std::string>());
        Server s(p, c);
        s.Start();
    } else if (result.count("c")) {
        if (result.count("i")) {
            options.parse_positional({"ip", "port"});
            auto icresult = options.parse(argc, argv);
            if (!icresult.count("ip") || !icresult.count("port"))
                std::cout << "Navedite ip adresu i port servera" << std::endl;

            Klijent k;
            k.InteractiveRun(icresult["ip"].as<std::string>(),
                             icresult["port"].as<std::string>());
        } else {
            options.parse_positional({"ip", "port", "com"});
            auto cresult = options.parse(argc, argv);
            if (!cresult.count("ip") || !cresult.count("port") ||
                !cresult.count("com"))
                std::cout << "Navedite ip adresu i port servera" << std::endl;
            std::string params;
            auto &v = cresult["com"].as<std::vector<std::string>>();
            for (const auto &s : v) {
                params += s;
                params += ' ';
            }
            params.pop_back();
            Klijent k;
            k.RunRegular(cresult["ip"].as<std::string>(),
                         cresult["port"].as<std::string>(), params);
        }
    }
}
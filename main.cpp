#include <iostream>

#include "cxxopts.hpp"
#include "klijent.h"
#include "server.h"

int main(int argc, char** argv) {
    cxxopts::Options options("KVStore", "Key value store");

    options.add_options()("s,server", "Start server", cxxopts::value<bool>()->default_value("false"))
        //
        ("c,cleint", "Start a client", cxxopts::value<bool>()->default_value("true"))
        //
        ("i,interactive", "Interactive option default set to true", cxxopts::value<bool>()->default_value("true"))
        //
        ("ip", "IP address", cxxopts::value<std::string>())
        //
        ("p,port", "Port number", cxxopts::value<std::string>())
        //
        ("com", "Command", cxxopts::value<std::vector<std::string>>())
        //
        ("con", "Nubmer of connections", cxxopts::value<std::string>())
        //
        ;

    auto result = options.parse(argc, argv);

    if (result.count("s")) {
        options.parse_positional({"p", "con"});
        auto sresult = options.parse(argc, argv);
        if (!sresult.count("p") || !sresult.count("con"))
            std::cout
                << "Navedite port i broj konekcija" << std::endl;
        //auto port = sresult["p"].as<std::string>();
        //auto con = sresult["con"].as<std::string>();
        uint16_t p = std::stoi(sresult["p"].as<std::string>());
        int c = std::stoi(sresult["con"].as<std::string>());
        Server s(p, c);
        s.Start();
    }
    if (result.count("c")) {
        if (result.count("i")) {
            options.parse_positional({"ip", "port"});
            auto icresult = options.parse(argc, argv);
            if (!icresult.count("ip") || !icresult.count("port"))
                std::cout << "Navedite ip adresu i port servera" << std::endl;
            //std::cout << icresult["ip"].as<std::string>() << " " << icresult["port"].as<std::string>() << std::endl;

            Klijent::InteractiveRun(icresult["ip"].as<std::string>(), icresult["port"].as<std::string>());

        } else {
            options.parse_positional({"ip", "port", "com"});
            auto cresult = options.parse(argc, argv);
            if (!cresult.count("ip") || !cresult.count("port") || !cresult.count("com"))
                std::cout << "Navedite ip adresu i port servera" << std::endl;
            std::string params;
            auto& v = cresult["com"].as<std::vector<std::string>>();
            for (const auto& s : v) {
                params += s;
                params += ' ';
            }
            params.pop_back();
            Klijent::RunRegular(cresult["ip"].as<std::string>(), cresult["port"].as<std::string>(), params);
        }
    }
    // if (result.count("s")) {
    //     std::cout << "Saw option ‘s’" << std::endl;
    // }
    // if (result.count("c")) {
    //     std::cout << "Saw option ‘c’" << std::endl;
    // }
    // if (result.count("i")) {
    //     std::cout << "Saw option ‘i’" << std::endl;
    // }
    // if (result.count("ip")) {
    //     std::cout << "ip = " << result["ip"].as<std::string>()
    //               << std::endl;
    // }

    // if (result.count("p")) {
    //     std::cout << "p = " << result["p"].as<std::string>()
    //               << std::endl;
    // }
    // if (result.count("com")) {
    //     std::cout << "Commands = {";
    //     auto& v = result["com"].as<std::vector<std::string>>();
    //     for (const auto& s : v) {
    //         std::cout << s << ", ";
    //     }
    //     std::cout << "}" << std::endl;
    // }
    // if (result.count("con")) {
    //     std::cout << "con = " << result["con"].as<std::string>()
    //               << std::endl;
    // }
}
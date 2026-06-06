#include <fstream>
#include <iostream>
#include <filesystem>
#include <regex>

/*
    quick hacky test program to just determine which spheres will be rendered. not really an injector but I liked the name. -ryu
*/

namespace fs = std::filesystem;
int main()
{
    fs::path p = fs::current_path();
    std::ifstream i{p / "script.cpp"};
    std::fstream o{p / "kernel_src.c", std::ios::in};

    std::string str((std::istreambuf_iterator<char>(i)), std::istreambuf_iterator<char>());
    std::string outfile((std::istreambuf_iterator<char>(o)), std::istreambuf_iterator<char>());

    std::regex r{R"(//INJECTOR.CPP CALL[\s\S]+//END INJECTOR.CPP CALL)"};
    std::string result = std::regex_replace(outfile, r, str);
    o.close();
    o.open(p / "kernel_src.c", std::ios::out);
    o << result;
}
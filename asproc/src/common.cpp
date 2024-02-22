#include "common.h"

#include <sstream>

long common::fileSize(const fs::path& filepath)
{
    std::ifstream ifs{filepath};
    long length{0};
    if(ifs)
    {
        ifs.seekg(0, ifs.end);
        length = ifs.tellg();
        ifs.seekg(0, ifs.beg);
    }
    return length;
}

void common::log(const std::string& msg)
{
    std::cout << msg << std::endl;
}

std::string common::variableName(std::string name)
{
    std::replace(name.begin(), name.end(), '.', '_');
    std::replace(name.begin(), name.end(), '-', '_');
    return name;
}

std::string common::floatToString(float f)
{
    std::ostringstream out;
    out << f;
    if(out.str().find('.') == std::string::npos)
    {
        out << ".0";
    }
    out << "f";
    return out.str();
}

void common::exportBytes(const std::vector<unsigned char>& bytes, std::ofstream& ofs)
{
    std::string delim{""};
    std::ios_base::fmtflags f( ofs.flags() );
    for(unsigned char c : bytes) {
        ofs << delim << "0x" << std::hex << std::setfill('0') << std::setw(2) << static_cast<int>(c);
        delim = ",";
    }
    ofs.flags( f );
}
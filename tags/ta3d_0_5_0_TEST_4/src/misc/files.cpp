
#include "files.h"
#include "../logs/logs.h"


namespace TA3D
{
namespace Paths
{
namespace Files
{


    template<class T>
    bool TmplLoadFromFile(T& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
    {
        if (emptyListBefore)
            out.clear();
        std::ifstream file(filename.c_str());
        if (!file)
        {
            LOG_WARNING("Impossible to open the file `" << filename << "`");
            return false;
        }
        if (sizeLimit)
        {
            file.seekg(0, std::ios_base::beg);
            std::ifstream::pos_type begin_pos = file.tellg();
            file.seekg(0, std::ios_base::end);
            if ((file.tellg() - begin_pos) > sizeLimit)
            {
                LOG_WARNING("Impossible to read the file `" << filename << "` (size > " << sizeLimit << ")");
                return false;
            }
            file.seekg(0, std::ios_base::beg);
        }
        String line;
        while (std::getline(file, line))
            out.push_back(line);
        return true;
    }
     
    bool Load(std::list<String>& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
    {
        return TmplLoadFromFile< std::list<String> >(out, filename, sizeLimit, emptyListBefore);
    }
     
    bool Load(std::vector<String>& out, const String& filename, const uint32 sizeLimit, const bool emptyListBefore)
    {
        return TmplLoadFromFile< std::vector<String> >(out, filename, sizeLimit, emptyListBefore);
    }
    



} // namespace Files
} // namespace Paths
} // namespace TA3D


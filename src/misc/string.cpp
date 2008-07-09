#include "string.h"

#if TA3D_USE_BOOST == 1 
#  include <boost/algorithm/string.hpp>
#  include <boost/algorithm/string/trim.hpp>
#  include <boost/algorithm/string/split.hpp>
#else
#  include <algorithm>
#endif


namespace TA3D
{

    #if TA3D_USE_BOOST != 1 
    static int stdLowerCase (int c)
    {
        return tolower(c);
    }
    
    static int stdUpperCase (int c)
    {
        return toupper(c);
    }
    #endif


    String&
    String::operator << (const wchar_t* v)
    {
        size_t l = wcslen(v);
        char* b = new char[l + 1];
        #ifndef WINDOWS
        wcstombs(&b[0], v, l);
        #else
        size_t i;
        wcstombs_s(&i, &b[0], l, v, l);
        #endif
        append(b);
        delete b;
        return *this;
    }

    String&
    String::toLower()
    {
        #if TA3D_USE_BOOST == 1 
        boost::to_lower(*this);
        #else
        std::transform (this->begin(), this->end(), this->begin(), stdLowerCase);
        #endif
        return *this;
    }

    String&
    String::toUpper()
    {
        #if TA3D_USE_BOOST == 1 
        boost::to_upper(*this);
        #else
        std::transform (this->begin(), this->end(), this->begin(), stdUpperCase);
        #endif
        return *this;
    }


    bool String::toBool() const
    {
        if (empty() || "0" == *this)
            return false;
        if ("1" == *this)
            return true;
        String s(*this);
        s.toLower();
        return ("true" == s || "on" == s);
    }

    String&
    String::trim(const String& trimChars)
    {
        // Find the first character position after excluding leading blank spaces
        std::string::size_type startpos = this->find_first_not_of(trimChars); 
        // Find the first character position from reverse af
        std::string::size_type endpos   = this->find_last_not_of(trimChars); 
 
        // if all spaces or empty return an empty string
        if ((std::string::npos == startpos) || (std::string::npos == endpos))
           *this = ""; 
        else
            *this = this->substr (startpos, endpos - startpos + 1);
        return *this;
    }

    void
    String::split(String::Vector& out, const String& separators, const bool emptyBefore) const
    {
        // Empty the container
        if (emptyBefore)
            out.clear();
        #if TA3D_USE_BOOST == 1 
        // TODO : Avoid string duplication
        // Split
        std::vector<std::string> v;
        boost::algorithm::split(v, *this, boost::is_any_of(separators.c_str()));
        // Copying
        for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i)
            out.push_back(*i);
        #else
        // TODO This method should be rewritten for better performance
        String s(*this);
        while (!s.empty())
        {                                                                                                            
            String::size_type i = s.find(separators);                                                                                    
            if (i == std::string::npos)                                                                                            
            {                                                                                                        
                out.push_back(String::Trim(s));                                                                        
                return;                                                                                              
            }
            else
            {                                                                                                        
                out.push_back(String::Trim(s.substr(0, i)));                                                          
                s = s.substr(i + 1, s.size() - i - 1);                                                               
            }                                                                                                        
        }
        #endif
    }

    void
    String::split(String::List& out, const String& separators, const bool emptyBefore) const
    {
        // Empty the container
        if (emptyBefore)
            out.clear();
        #if TA3D_USE_BOOST == 1 
        // TODO : Avoid string duplication
        // Split
        std::vector<std::string> v;
        boost::algorithm::split(v, *this, boost::is_any_of(separators.c_str()));
        // Copying
        for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i)
            out.push_back(*i);
        #else
        // TODO This method should be rewritten for better performance
        String s(*this);
        while (!s.empty())
        {                                                                                                            
            String::size_type i = s.find(separators);                                                                                    
            if (i == std::string::npos)                                                                                            
            {                                                                                                        
                out.push_back(String::Trim(s));                                                                        
                return;                                                                                              
            }
            else
            {                                                                                                        
                out.push_back(String::Trim(s.substr(0, i)));                                                          
                s = s.substr(i + 1, s.size() - i - 1);                                                               
            }                                                                                                        
        }
        #endif
    }


    void String::ToKeyValue(const String& s, String& key, String& value)
    {
        String::size_type pos = s.find_first_not_of(TA3D_WSTR_SEPARATORS);
        if (pos == String::npos)
        {
            // The string is empty
            key.clear();
            value.clear();
            return;
        }
        // =
        String::size_type equal = s.find_first_of('=', pos);
        if (equal == String::npos)
        {
            value.clear();
            if (s[pos] == '[')
            {
                key = s.substr(pos, s.find_first_of(']', pos));
                return;
            }
            String::size_type end = s.find_last_not_of(TA3D_WSTR_SEPARATORS);
            key = s.substr(pos, end - pos + 1);
            return;
        }
        key = s.substr(pos, equal - 1);
        // ;
        String::size_type semicolon = s.find_last_not_of(';');
        if (semicolon == String::npos)
        {
            value.clear();
            return;
        }
        // Remove spaces
        semicolon = s.find_last_not_of(TA3D_WSTR_SEPARATORS, semicolon - 1);
        equal = s.find_first_not_of(TA3D_WSTR_SEPARATORS, equal);
        value = s.substr(equal + 1, semicolon - equal + 1);
    }


    String& String::convertAntiSlashesIntoSlashes()
    {
        for (String::iterator i = this->begin(); i != this->end(); ++i)
        {
            if (*i == '\\')
                *i = '/';
        }
        return *this;
    }

    String& String::convertSlashesIntoAntiSlashes()
    {
        for (String::iterator i = this->begin(); i != this->end(); ++i)
        {
            if (*i == '/')
                *i = '\\';
        }
        return *this;
    }


}

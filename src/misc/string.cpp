#include "../stdafx.h"
#include "string.h"
#include <allegro.h>

#if TA3D_USE_BOOST == 1 
#  include <boost/algorithm/string.hpp>
#  include <boost/algorithm/string/trim.hpp>
#  include <boost/algorithm/string/split.hpp>
#else
#  include <algorithm>
#endif
#include "../logs/logs.h"



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
            *this = this->substr(startpos, endpos - startpos + 1);
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


    void String::ToKeyValue(const String& s, String& key, String& value, const enum String::CharCase chcase)
    {
        // The first usefull character
        String::size_type pos = s.find_first_not_of(TA3D_WSTR_SEPARATORS);
        if (pos == String::npos)
        {
            // The string is empty
            key.clear();
            value.clear();
            return;
        }
        // The first `=` character 
        String::size_type equal = s.find_first_of('=', pos);
        if (equal == String::npos)
        {
            // If none is present, it may be a section statement
            if (s[pos] == '[')
            {
                key = "[";
                pos = s.find_first_not_of(TA3D_WSTR_SEPARATORS, pos + 1);
                String::size_type end = s.find_first_of(']', pos);
                end = s.find_last_not_of(TA3D_WSTR_SEPARATORS, end - 1);
                value = s.substr(pos, end - pos + 1);
                return;
            }
            // otherwise it is only a string
            value.clear();
            // But it may be a comment
            String::size_type slashes = s.find("//", pos);
            if (pos == slashes)
            {
                key.clear();
                return;
            }
            String::size_type end = s.find_last_not_of(TA3D_WSTR_SEPARATORS, slashes - 1);
            key = s.substr(pos, end - pos + 1);
            return;
        }

        // We can extract our key
        String::size_type end = s.find_last_not_of(TA3D_WSTR_SEPARATORS, equal - 1);
        key = s.substr(pos, 1 + end - pos);
        String::size_type slashes = key.rfind("//");
        // Remove any comments
        if (slashes != String::npos)
        {
            value.clear();
            if (slashes == 0) // the key is a comment actually
                key.clear();
            else
            {
                // Get only the good part
                slashes = key.find_last_not_of(TA3D_WSTR_SEPARATORS, slashes - 1);
                key = key.substr(0, slashes + 1);
                if (chcase == soIgnoreCase)
                    key.toLower();
            }
            return;
        }
        if (chcase == soIgnoreCase)
            key.toLower();

        // Left-Trim for the value
        equal = s.find_first_not_of(TA3D_WSTR_SEPARATORS, equal + 1);
        if (String::npos == equal)
        {
            value.clear();
            return;
        }

        // Looking for the first semicolon
        String::size_type semicolon = s.find_first_of(';', equal);
        if (semicolon == String::npos)
        {
            // if none is present, looks for a comment to strip it
            String::size_type slashes = s.find("//", equal);
            slashes = s.find_last_not_of(TA3D_WSTR_SEPARATORS, slashes - 1);
            value = s.substr(equal, 1 + slashes - equal);
            value.findAndReplace("\\r", "", soCaseSensitive);
            value.findAndReplace("\\n", "\n", soCaseSensitive);
            return;
        }
        // Remove spaces before the semicolon and after the `=`
        semicolon = s.find_last_not_of(TA3D_WSTR_SEPARATORS, semicolon - 1);

        // We can extract the value
        value = s.substr(equal, 1 + semicolon - equal);
        value.findAndReplace("\\r", "", soCaseSensitive);
        value.findAndReplace("\\n", "\n", soCaseSensitive);
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

    uint32 String::hashValue() const
    {
        uint32 hash = 0;
        for (String::const_iterator i = this->begin(); i != this->end(); ++i)
            hash = (hash << 5) - hash + *i;
        return hash;
    }

    int String::FindInList(const String::Vector& l, const char* s)
    {
        int indx(0);
        for (String::Vector::const_iterator i = l.begin(); i != l.end(); ++i, ++indx)
        {
            if(s == *i)
                return indx;
        }
        return -1;
    }


    int String::FindInList(const String::Vector& l, const String& s)
    {
        int indx(0);
        for (String::Vector::const_iterator i = l.begin(); i != l.end(); ++i, ++indx)
        {
            if(s == *i)
                return indx;
        }
        return -1;
    }


    char* String::ConvertToUTF8(const char* s)
    {
        if (NULL != s && *s != '\0')
            return ConvertToUTF8(s, strlen(s));
        char* ret = new char[1];
        LOG_ASSERT(NULL != ret);
        *ret = '\0';
        return ret;
    }


    char* String::ConvertToUTF8(const char* s, uint32 len)
    {
        if (NULL == s || '\0' == *s)
        {
            char* ret = new char[1];
            LOG_ASSERT(NULL != ret);
            *ret = '\0';
            return ret;
        }
        // see http://linux.die.net/man/3/uconvert_size
        const uint32 newSize = uconvert_size(s, U_ASCII, U_UTF8); // including the mandatory zero terminator of the string
        char* ret = new char[newSize];
        LOG_ASSERT(NULL != ret);
        // see http://linux.die.net/man/3/do_uconvert
        do_uconvert(s, U_ASCII, ret, U_UTF8, newSize);
        ret[newSize] = '\0'; // A bit paranoid 
        return ret;
    }


    String String::ConvertToUTF8(const String& s)
    {
        if (s.empty())
            return String();
        char* ret = ConvertToUTF8(s.c_str(), s.size());
        if (ret)
        {
            String s(ret); // TODO Find a way to not use a temporary string
            delete[] ret;
            return s;
        }
        return String();
    }


    String& String::findAndReplace(char toSearch, const char replaceWith, const enum String::CharCase option)
    {
        if (option == soIgnoreCase)
        {
            toSearch = tolower(toSearch);
            for (String::iterator i = this->begin(); i != this->end(); ++i)
            {
                if (tolower(*i) == toSearch)
                    *i = replaceWith;
            }
        }
        else
        {
            for (String::iterator i = this->begin(); i != this->end(); ++i)
            {
                if (*i == toSearch)
                    *i = replaceWith;
            }

        }
        return *this;
    }


    String& String::findAndReplace(const String& toSearch, const String& replaceWith, const enum String::CharCase option)
    {
        if (soCaseSensitive == option)
        {
            String::size_type p = 0;
            String::size_type siz = toSearch.size();
            while ((p = this->find(toSearch, p)) != String::npos)
                this->replace(p, siz, replaceWith);
        }
        else
        {
            *this = String::ToLower(*this).findAndReplace(String::ToLower(toSearch), replaceWith, soCaseSensitive);
        }
        return *this;
    }

    /*
    template<typename T>
    int stringConvert(const String& section, const T t, const T expected)
    {
        if (t != expected)
        {
            LOG_ERROR("* String convertion failed !");
            LOG_ERROR("     From `String` to `" << section << "`");
            LOG_ERROR("     Expected: `" << expected << "`, got `" << t << "`");
            return 1;
        }
        return 0;
    }

    static int stringCheckKV(const String& t, const String& expectedKey = "", const String& expectedValue = "")
    {
        String k;
        String v;
        TA3D::String::ToKeyValue(t, k, v);
        if (k != expectedKey || v != expectedValue)
        {
            LOG_ERROR("* String Check failed !");
            LOG_ERROR("     On `" << t << "`");
            LOG_ERROR("     Expectd: key=`" << expectedKey << "`,  value=`" << expectedValue << "`");
            LOG_ERROR("     Found: key=`" << k << "`,  value=`" << v << "`");
            return 1;
        }
        return 0;
    }


    int String::AutoTest()
    {
        int ret = 0;
        ret += stringConvert("int32", String(10).toInt32(), 10);
        ret += stringConvert("uint32", String(uint32(-1)).toUInt32(), uint32(-1));
        ret += stringConvert("int64", String(sint64(33554432 * 30)).toInt64(), sint64(33554432 * 30));
        ret += stringConvert("hexa", String("0xFF").toInt32(), (sint32)255);
        ret += stringCheckKV("a=b;", "a", "b");
        ret += stringCheckKV(" a=b;", "a", "b");
        ret += stringCheckKV("a= b;", "a", "b");
        ret += stringCheckKV("a=b; ", "a", "b");
        ret += stringCheckKV(" a = b ;", "a", "b");
        ret += stringCheckKV(" !(*^&$% = b ;", "!(*^&$%", "b");
        ret += stringCheckKV(" ab c = 1 2 3 4 5 ;", "ab c", "1 2 3 4 5");
        ret += stringCheckKV(" foo=bar; // comments here; ", "foo", "bar");
        ret += stringCheckKV(" foo=bar comments//here ", "foo", "bar comments");
        ret += stringCheckKV(" foo=bar comments // here ", "foo", "bar comments");
        ret += stringCheckKV(" foo=bar  /   this is not a comment    ", "foo", "bar  /   this is not a comment");
        ret += stringCheckKV("foo=http://www.example.org;// comment", "foo", "http://www.example.org");
        ret += stringCheckKV("[ini] ", "[", "ini");
        ret += stringCheckKV("  [Example of Section] ", "[", "Example of Section");
        ret += stringCheckKV("  [ foo section  ] ", "[", "foo section");
        ret += stringCheckKV("  [ bad section ", "[", "bad section");
        ret += stringCheckKV("  [ bad section // comments", "[", "bad section // comments");
        ret += stringCheckKV("[ bad section // comments  ", "[", "bad section // comments");
        ret += stringCheckKV("  [Example of Section] // Comments", "[", "Example of Section");
        ret += stringCheckKV("  [Example of Section]//Comments", "[", "Example of Section");
        ret += stringCheckKV("  // Here is a comment ");
        ret += stringCheckKV("  nyo // Here is a comment ", "nyo");
        ret += stringCheckKV("  // a = b; ");
        ret += stringCheckKV("  / a = b; ", "/ a", "b");
        ret += stringCheckKV("  piko//li = b; ", "piko", "");
        ret += stringCheckKV("  piko/li = b; ", "piko/li", "b");
        ret += stringCheckKV("  piko   //li = b; ", "piko", "");
        ret += stringCheckKV("{ ", "{");
        ret += stringCheckKV("  }", "}");
        ret += stringCheckKV("   { // Start of a new block", "{");
        ret += stringCheckKV(" } // end of block", "}");
        ret += stringCheckKV(" a = a full text with \\n cariage return", "a", "a full text with \n cariage return");
        return ret;
    }
    */

}

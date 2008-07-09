#ifndef __TA3D_LIB_W_STRING_H__
# define __TA3D_LIB_W_STRING_H__

# include <sstream>
# include <list>
# include <vector>

/*!
** \brief Force the Use of the Boost functions 
*/
# define TA3D_USE_BOOST  0



//! \name Macros for TA3D::String
//@{

//! Macro to append some data to the string
# define TA3D_WSTR_APPEND      std::stringstream out; \
                                out << v; \
                                append(out.str());\
                                return *this

//! Macro to convert the string into a given type
# define TA3D_WSTR_CAST_OP(X)  X v; \
                                fromString<X>(v, *this, std::dec); \
                                return v;

//! Macro to append the value of a boolean (true -> "true", false -> "false")
# define TA3D_WSTR_APPEND_BOOL(X)   append(X ? "true" : "false")

//@} // Macros for TA3D::String


# define TA3D_WSTR_SEPARATORS  " \t\r\n"



namespace TA3D
{

    /*! \class String
    ** \brief A String implementation for `Worlds`
    **
    ** \code
    **      TA3D::String a("abcd");
    **      std::cout << a << std::endl;  // display: `abcd`
    **      TA3D::String b(10 + 2);
    **      std::cout << b << std::endl;  // display: `12`
    **      TA3D::String c(10.3);
    **      std::cout << c << std::endl;  // display: `10.3`
    **
    **      // The same with the operator `<<`
    **      TA3D::String d;
    **      d << "Value : " << 42;
    **      std::cout << d << std::endl;  // display: `Value : 42`
    ** \endcode
    **
    ** Here is an example to show when to use static methods :
    ** \code
    **      TA3D::String s = "HelLo wOrLd";
    **      std::cout << TA3D::String::ToLower(s) << std::endl;  // `hello world`
    **      std::cout << s << std::endl;  // `HelLo wOrLd`
    **      std::cout << s.toLower() << std::endl;  // `hello world`
    **      std::cout << s << std::endl;  // `hello world`
    ** \endcode
    */
    class String : public std::string
    {
    public:
        //! A String list
        typedef std::list<String> List;
        //! A String vector
        typedef std::vector<String> Vector;

    public:
        /*!
        ** \brief Copy then Convert the case (lower case) of characters in the string using the UTF8 charset
        ** \param s The string to convert
        ** \return A new string
        */
        static String ToLower(const char* s) {return String(s).toLower();}
        /*!
        ** \brief Copy then Convert the case (lower case) of characters in the string using the UTF8 charset
        ** \param s The string to convert
        ** \return A new string
        */
        static String ToLower(const wchar_t* s) {return String(s).toLower();}
        /*!
        ** \brief Copy then Convert the case (lower case) of characters in the string using the UTF8 charset
        ** \param s The string to convert
        ** \return A new string
        */
        static String ToLower(const String& s) {return String(s).toLower();}

        /*!
        ** \brief Copy then Convert the case (upper case) of characters in the string using the UTF8 charset
        ** \param s The string to convert
        ** \return A new string
        */
        static String ToUpper(const char* s) {return String(s).toUpper();}
        /*!
        ** \brief Copy then Convert the case (upper case) of characters in the string using the UTF8 charset
        ** \param s The string to convert
        ** \return A new string
        */
        static String ToUpper(const wchar_t* s) {return String(s).toUpper();}
        /*!
        ** \brief Copy then Convert the case (upper case) of characters in the string using the UTF8 charset
        ** \param s The string to convert
        ** \return A new string
        */
        static String ToUpper(const String& s) {return String(s).toUpper();}

        /*!
        ** \brief Remove trailing and leading spaces
        ** \param s The string to convert
        ** \param trimChars The chars to remove
        ** \return A new string
        */
        static String Trim(const char* s, const String& trimChars = TA3D_WSTR_SEPARATORS) {return String(s).trim(trimChars);}
        /*!
        ** \brief Remove trailing and leading spaces
        ** \param s The string to convert
        ** \param trimChars The chars to remove
        ** \return A new string
        */
        static String Trim(const wchar_t* s, const String& trimChars = TA3D_WSTR_SEPARATORS) {return String(s).trim(trimChars);}
        /*!
        ** \brief Remove trailing and leading spaces
        ** \param s The string to convert
        ** \param trimChars The chars to remove
        ** \return A new string
        */
        static String Trim(const String& s, const String& trimChars = TA3D_WSTR_SEPARATORS) {return String(s).trim(trimChars);}

        /*!
        ** \brief Convert all antislashes into slashes
        ** \param[in,out] s The string to convert
        ** \return A new string
        */
        static String ConvertAntiSlashesIntoSlashes(String& s) {return String(s).convertAntiSlashesIntoSlashes();}

        /*!
        ** \brief Convert all slashes into antislashes
        ** \param[in,out] s The string to convert
        ** \return A new string
        */ 
        static String ConvertSlashesIntoAntiSlashes(String& s) {return String(s).convertSlashesIntoAntiSlashes();}

        /*!
        ** \brief Extract the key and its value from a string (mainly provided by TDF files)
        **
        ** \param s A line (ex: `   category=core vtol ctrl_v level1 weapon  notsub ;`)
        ** \param[out] key The key that has been found
        ** \param[out] value The associated value
        **
        ** \code
        **    String k, v;
        **
        **    // -> k='category'
        **    // -> v='core vtol ctrl_v level1 weapon  notsub'
        **    String::ToKeyValue("  category=core vtol ctrl_v level1 weapon  notsub ;", k, v)
        **
        **    // -> k='foo'
        **    // -> v='bar'
        **    String::ToKeyValue("  foo  = bar ; ");
        **
        **    // -> k='}'  v=''
        **    String::ToKeyValue("  } ", k, v);
        ** \endcode
        */
        static void ToKeyValue(const String& s, String& key, String& value);

    public:
        //! \name Constructors and Destructor
        //@{
        //! Default constructor
        String() :std::string() {}
        //! Constructor by copy
        String(const String& v, size_type pos = 0, size_type n = npos) :std::string(v, pos, n) {}
        //! Constructor with a default value from a std::string
        String(const std::string& v) :std::string(v) {}
        //! Constructor with a default value from a wide string (wchar_t*)
        String(const wchar_t* v) :std::string() {*this << v;}
        //! Constructor with a default value from a string (char*)
        String(const char* v) :std::string(v) {}
        //! Constructor with a default value from a string (char*) and a length
        String(const char* v, String::size_type n) :std::string(v, n) {}
        //! Constructor with a default value from a single char
        String(const char v) :std::string() {*this += v;}
        //! Constructor with a default value from an int (8 bits)
        String(const int8_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an int (16 bits)
        String(const int16_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an int (32 bits)
        String(const int32_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an int (64 bits)
        String(const int64_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an unsigned int (8 bits)
        String(const uint8_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an unsigned int (16 bits)
        String(const uint16_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an unsigned int (32 bits)
        String(const uint32_t v) :std::string() {*this << v;}
        //! Constructor with a default value from an unsigned int (64 bits)
        String(const uint64_t v) :std::string() {*this << v;}
        //! Constructor with a default value from a float
        String(const float v) :std::string() {*this << v;}
        //! Constructor with a default value from a double
        String(const double v) :std::string() {*this << v;}
        //! Destructor
        ~String() {}
        //@}


        //! \name The operator `<<`
        //@{
        //! Append a string (char*) 
        String& operator << (const char* v) {append(v);return *this;}
        //! Append a string (stl)
        String& operator << (const std::string& v) {append(v);return *this;}
        //! Append a string (TA3D::String)
        String& operator << (const String& v) {append(v);return *this;}
        //! Append a single char
        String& operator << (const char v) {*(static_cast<std::string*>(this)) += v; return *this;}
        //! Append a wide string (wchar_t*)
        String& operator << (const wchar_t* v);
        //! Append an int (8 bits)
        String& operator << (const int8_t v) {TA3D_WSTR_APPEND;}
        //! Append an unsigned int (8 bits)
        String& operator << (const uint8_t v) {TA3D_WSTR_APPEND;}
        //! Append an int (16 bits)
        String& operator << (const int16_t v) {TA3D_WSTR_APPEND;}
        //! Append an unsigned int (16 bits)
        String& operator << (const uint16_t v) {TA3D_WSTR_APPEND;}
        //! Append an int (32 bits)
        String& operator << (const int32_t v) {TA3D_WSTR_APPEND;}
        //! Append an unsigned int (32 bits)
        String& operator << (const uint32_t v) {TA3D_WSTR_APPEND;}
        //! Append an int (64 bits)
        String& operator << (const int64_t v) {TA3D_WSTR_APPEND;}
        //! Append an unsigned int (64 bits)
        String& operator << (const uint64_t v) {TA3D_WSTR_APPEND;}
        //! Convert then Append a float
        String& operator << (const float v) {TA3D_WSTR_APPEND;}
        //! Convert then Append a double 
        String& operator << (const double v) {TA3D_WSTR_APPEND;}
        //! Convert then Append a boolean (will be converted into "true" or "false")
        String& operator << (const bool v) {TA3D_WSTR_APPEND_BOOL(v);return *this;}
        //@}


        //! \name Convertions
        //@{
        //! Convert this string into an int (8 bits)
        int8_t toInt8() const {TA3D_WSTR_CAST_OP(int8_t);} 
        //! Convert this string into an int (16 bits)
        int16_t toInt16() const {TA3D_WSTR_CAST_OP(int16_t);} 
        //! Convert this string into an int (32 bits)
        int32_t toInt32() const {TA3D_WSTR_CAST_OP(int32_t);} 
        //! Convert this string into an int (64 bits)
        int64_t toInt64() const {TA3D_WSTR_CAST_OP(int64_t);} 
        //! Convert this string into an unsigned int (8 bits)
        uint8_t toUInt8() const {TA3D_WSTR_CAST_OP(uint8_t);} 
        //! Convert this string into an unsigned int (16 bits)
        uint16_t toUInt16() const {TA3D_WSTR_CAST_OP(uint16_t);} 
        //! Convert this string into an unsigned int (32 bits)
        uint32_t toUInt32() const {TA3D_WSTR_CAST_OP(uint32_t);} 
        //! Convert this string into an unsigned int (64 bits)
        uint64_t toUInt64() const {TA3D_WSTR_CAST_OP(uint64_t);} 
        //! Convert this string into a float
        float toFloat() const {TA3D_WSTR_CAST_OP(float);} 
        //! Convert this string into a double
        double toDouble() const {TA3D_WSTR_CAST_OP(double);} 
        //! Convert this string into a bool (true if the lower case value is equals to "true", "1" or "on")
        bool toBool() const; 

        //! \name The operator `+=` (with the same abilities than the operator `<<`)
        //@{
        //! Append a string (char*) 
        String& operator += (const char* v) {append(v); return *this;}
        //! Append a string (stl)
        String& operator += (const std::string& v) {append(v); return *this;}
        //! Append a string (TA3D::String)
        String& operator += (const TA3D::String& v) {append(v); return *this; }
        //! Append a single char
        String& operator += (const char v) {*(static_cast<std::string*>(this)) += (char)v; return *this; return *this;}
        //! Append a wide string (wchar_t*)
        String& operator += (const wchar_t* v) {*this << v; return *this;}
        //! Append an int (8 bits)
        String& operator += (const int8_t v) {*this << v; return *this;}
        //! Append an unsigned int (8 bits)
        String& operator += (const uint8_t v) {*this << v; return *this;}
        //! Append an int (16 bits)
        String& operator += (const int16_t v) {*this << v; return *this; }
        //! Append an unsigned int (16 bits)
        String& operator += (const uint16_t v) {*this << v; return *this; }
        //! Append an int (32 bits)
        String& operator += (const int32_t v) {*this << v; return *this; }
        //! Append an unsigned int (32 bits)
        String& operator += (const uint32_t v) {*this << v; return *this; }
        //! Append an int (64 bits)
        String& operator += (const int64_t v) {*this << v; return *this; }
        //! Append an unsigned int (64 bits)
        String& operator += (const uint64_t v) {*this << v; return *this; }
        //! Convert then Append a float
        String& operator += (const float v) {*this << v; return *this; }
        //! Convert then Append a double 
        String& operator += (const double v) {*this << v; return *this; }
        //! Convert then Append a boolean (will be converted into "true" or "false")
        String& operator += (const bool v) {TA3D_WSTR_APPEND_BOOL(v); return *this; }
        //@}


        //! \name The operator `+`
        //@{
        //! Create a new String from the concatenation of *this and a string
        const String operator + (const String& rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a char
        const String operator + (const char rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a widechar
        const String operator + (const wchar_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a const char*
        const String operator + (const char* rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a wide string
        const String operator + (const wchar_t* rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a signed int (8 bits)
        const String operator + (const int8_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a signed int (16 bits)
        const String operator + (const int16_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a signed int (32 bits)
        const String operator + (const int32_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a signed int (64 bits)
        const String operator + (const int64_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and an unsigned int (8 bits)
        const String operator + (const uint8_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and an unsigned int (16 bits)
        const String operator + (const uint16_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and an unsigned int (32 bits)
        const String operator + (const uint32_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and an unsigned int (64 bits)
        const String operator + (const uint64_t rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a float
        const String operator + (const float rhs) { return String(*this) += rhs; }
        //! Create a new String from the concatenation of *this and a double
        const String operator + (const double rhs) { return String(*this) += rhs; }
        //@}


        //! \name Case convertion
        //@{
        /*!
        ** \brief Convert the case (lower case) of characters in the string using the UTF8 charset
        ** \return Returns *this
        */
        String& toLower();
        /*!
        ** \brief Convert the case (upper case) of characters in the string using the UTF8 charset
        ** \return Returns *this
        */
        String& toUpper();
        //@} Case convertion


        /*!
        ** \brief Remove trailing and leading spaces
        ** \param trimChars The chars to remove
        ** \return Returns *this
        */
        String& trim(const String& trimChars = TA3D_WSTR_SEPARATORS);


        //! \name Split
        //@{

        /*!
        ** \brief Divide a string into several parts
        ** \param[out] All found parts
        ** \param sepearators Sequence of chars considered as a separator
        ** \param emptyBefore True to clear the vector before fulfill it
        ** \warning Do not take care of string representation (with `'` or `"`)
        */
        void split(String::Vector& out, const String& separators = TA3D_WSTR_SEPARATORS, const bool emptyBefore = true) const;
        /*!
        ** \brief Divide a string into several parts
        ** \param[out] All found parts
        ** \param sepearators Sequence of chars considered as a separator
        ** \param emptyBefore True to clear the list before fulfill it
        ** \warning Do not take care of string representation (with `'` or `"`)
        */
        void split(String::List& out, const String& separators = TA3D_WSTR_SEPARATORS, const bool emptyBefore = true) const;

        //@} Split


        /*!
        ** \brief Extract the key and its value from a string (mainly provided by TDF files)
        **
        ** \param[out] key The key that has been found
        ** \param[out] value The associated value
        **
        ** \see String::ToKeyValue()
        */
        void toKeyValue(String& key, String& value) const { ToKeyValue(*this, key, value); }

        /*!
        ** \brief Convert all antislashes into slashes
        ** \return Returns *this
        */
        String& convertAntiSlashesIntoSlashes();

        /*!
        ** \brief Convert all slashes into antislashes
        ** \return Returns *this
        */
        String& convertSlashesIntoAntiSlashes();


    private:

        /*!
        ** \brief Convert a string into another type, given by the template parameter `T`
        ** \param[out] t The destination variable
        ** \param s The string to convert
        ** \param f The base to use for the convertion (std::hex, std::dec, ...)
        ** \return True if the operation succeeded, False otherwise
        */
        template <class T>
        bool fromString(T& t, const String& s, std::ios_base& (*f)(std::ios_base&)) const
        {
            std::istringstream iss(s);
            return !(iss >> f >> t).fail();
        }

    }; // class String





} // namespace TA3D

#endif // __TA3D_LIB_W_STRING_H__

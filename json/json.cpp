#include "json.h"
#include <fstream>

namespace
{
    static int __lineCount{0};    
}

json::Json::Json() : Json{Unassigned}
{

}

json::Json::Json(const json::Type& type) : Json{type, "", ""}
{

}

json::Json::Json(const json::Type& type, const std::string& key, const std::string& value)
    : _type{type}, _key{key}, _value{value}
{

}

json::Json::~Json() noexcept
{
    clear();
}

json::Json& json::Json::fromFile(const std::filesystem::path& filename)
{
    std::ifstream ifs{filename};
    if(!ifs.is_open())
    {
        throw std::runtime_error("could not open file " + filename.string());
    }
    ifs >> *this;
    return *this;
}

void assertKey(const std::string& key)
{
    if(key.length() <= 0)
    {
        throw std::runtime_error("key is empty.");        
    }
}

void json::Json::add(const std::string& key, Json& obj)
{
    if(!isObject())
    {
        throw std::runtime_error("trying to add parameter to a non-object");
    }
    if(obj.isElement())
    {
        throw std::runtime_error("trying to add an element to an object");
    }
    obj._parent = this;
    _children.emplace(key, obj);
}

void json::Json::insert(Json& obj)
{
    if(!isArray())
    {
        throw std::runtime_error("trying to insert element to a non-array");
    }
    if(obj.isParameter())
    {
        throw std::runtime_error("trying to add an parameter to an array");
    }
    obj._parent = this;
    _array.push_back(obj);
}

enum class State
{
    ParseKey,
    ParseValue,
    ParseElements
};

void json::Json::parse(std::istream& is, json::Json& obj)
{
    char c;
    char startchar{0};
    char endchar{0};
    bool parseString{false};
    std::string stringValue{""};
    std::string key{""};
    State state{State::ParseKey};
    bool escape{false};

    auto handleStart = [&](char start, char end) {
        if(startchar == 0)
        {
            if(start == '{')
            {
                state = State::ParseKey;
            }
            else if(start == '[')
            {
                state = State::ParseElements;
            }
            startchar = start;
            endchar = end;
        }
        else
        {
            json::Json anotherOne{start == '{' ? json::Object : json::Array, key, ""};
            obj.parse(is, anotherOne);
            if(startchar == '{')
            {
                obj.add(key, anotherOne);
            }
            else if(startchar == '[')
            {
                obj.insert(anotherOne);
            }
        }
    };

    while((c = static_cast<char>(is.peek())) != EOF && !is.fail())
    {
        //std::cout << c;

        if(parseString)
        {
            if(c == '"' && !escape)
            {
                parseString = false;
                switch(state)
                {
                    case State::ParseKey:
                        key = stringValue;
                        // state transition handled in ':' case
                        break;
                    case State::ParseValue:
                    {
                        assertKey(key);
                        Json strElem{json::Parameter, key, stringValue};
                        strElem._stringType = true;
                        obj.add(key, strElem);
                        key = "";
                        state = State::ParseKey;
                        break;
                    }
                    case State::ParseElements:
                    {
                        Json strElem{json::Element, "", stringValue};
                        strElem._stringType = true;
                        obj.insert(strElem);
                        break;
                    }
                }
                stringValue = "";
            }
            else
            {
                if(c == '\\' && !escape)
                {
                    escape = true;
                }
                else
                {
                    stringValue += c;
                    escape = false;
                }
            }
        }
        else
        {
            switch(c)
            {
                case '{':
                    if(_type == json::Unassigned)
                    {
                        _type = json::Object;
                    }
                    handleStart('{', '}');
                    break;
                case '[':
                    if(_type == json::Unassigned)
                    {
                        _type = json::Array;
                    }
                    handleStart('[', ']');
                    break;
                case '"':
                    parseString = true;
                    break;
                case ':':
                    assertKey(key);
                    state = State::ParseValue;
                    break;
                case ',':
                    switch(state)
                    {
                        case State::ParseElements:
                        {
                            if(stringValue.length() > 0) // Probably already handled in string parsing.
                            {
                                Json elementObj{json::Element, "", stringValue};
                                obj.insert(elementObj);
                                stringValue = "";
                            }
                            break;
                        }
                        case State::ParseValue:
                        {
                            assertKey(key);
                            Json valueObj{json::Parameter, key, stringValue};
                            obj.add(key, valueObj);
                            stringValue = "";
                            state = State::ParseKey;
                            break;
                        }
                        default:
                            break;
                    }
                    break;
                case '}':
                case ']':
                    if(endchar != c)
                    {
                        throw std::runtime_error("unexpected endchar.");
                    }
                    switch(state)
                    {
                        case State::ParseValue:
                            if(stringValue.size() > 0)
                            {
                                assertKey(key);
                                Json valueObj{json::Parameter, key, stringValue};
                                obj.add(key, valueObj);
                                stringValue = "";
                                state = State::ParseKey;
                            }
                            break;
                        case State::ParseElements:
                            if(stringValue.size() > 0)
                            {
                                Json elementObj{json::Element, "", stringValue};
                                obj.insert(elementObj);
                                stringValue = "";
                                state = State::ParseKey;
                            }
                            break;
                        case State::ParseKey:
                            //throw std::runtime_error("unexpected endchar during ParseKey.");
                            break;
                    }
                    //is.get();
                    return;
                    break;
                case '\n':
                    ++__lineCount;
                case '\r':
                case '\t':
                case ' ':
                    break;
                default:
                    switch(state)
                    {
                        case State::ParseValue:
                        case State::ParseElements:
                            stringValue += c;
                            break;
                        default:
                            throw std::runtime_error("Unexpected character.");
                            break;
                    }
                    break;
            }
        }
        is.get();
    }
}

std::istream& json::operator>>(std::istream& is, json::Json& obj)
{
    __lineCount = 0;
    try
    {
        obj.parse(is, obj);
    }
    catch(const std::exception& e)
    {
        std::cout << "Error at line " << ++__lineCount << ": " << e.what() << std::endl;
    }
    return is;
}

void json::Json::print(std::ostream& os, std::string indent) const
{
    std::string delim{""};
    switch(_type)
    {
        case json::Unassigned:
            os << "Unassigned";
            break;
        case json::Object:
            os << "{" << std::endl;
            for(auto& pair : _children)
            {
                os << delim << indent << "  \"" << pair.first << "\": ";
                pair.second.print(os, indent + "  ");
                delim = ",\n";
            }
            os << std::endl << indent << "}";
            break;
        case json::Array:
            os << "[" << std::endl;
            for(auto& child : _array)
            {
                os << delim << indent << "  ";
                child.print(os, indent + "  ");
                delim = ",\n";
            }
            os << std::endl << indent << "]";
            break;
        case json::Parameter:
        case json::Element:
            if(_stringType)
            {
                os << '"' << _value << '"';
            }
            else
            {
                os << _value;
            }
            break;
        default:
            throw(std::runtime_error("Unknown type."));
    }
}

std::ostream& json::operator<<(std::ostream& os, const json::Json& obj)
{
    try
    {
        obj.print(os, "");
    }
    catch(const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    return os;
}

#include <Key.h>


bool Key::isRegularKey() const
{
    /* Between space and ~ */
    if(key_code >= 32 && key_code <= 126)
        return true;
    return false;
}

bool Key::isRegularKey() const
{
    return (key_code != -1);
}

void Key::fromNCurses(int ch)
{
    /* TODO: ?? */
    key_code = ch;
}

char Key::getChar() const
{
    return (char)key_code;
}

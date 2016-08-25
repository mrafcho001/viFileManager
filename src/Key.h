#ifndef KEY_H
#define KEY_H


class Key
{
public:

    /* Is the key one of the normal printable keyboard keys:
     * Alphanumerics, symbols */
    bool isRegularKey() const;

    bool isValid() const;

    bool isNumeric() const;

    void fromNCurses(int ch);

    char getChar() const;

private:
    int key_code{-1};
};

#endif

#ifndef COMMANDENGINE_H
#define COMMANDENGINE_H
#include <functional>
#include <memory>
#include "Key.h"


/* Command Engine - Process input and make calls to registered callbacks */

class CommandEngine
{
public:

    void processInput(const Key &input);


    /* Register basic arrow movements: up, down, left, and right
     * the callback function should accept single int parameter indicating
     * the repetition count of the movement */
    void registerLeftMove(std::function<void(int)> &f);
    void registerRightMove(std::function<void(int)> &f);
    void registerUpMove(std::function<void(int)> &f);
    void registerDownMove(std::function<void(int)> &f);

    /* Whenever an invalid key is entered for the current state, this CB is
     * invoked, key processing will begin fresh with next key */
    void registerInvalidCommand(std::function<void(void)> &f);

private:

    /* State Machine */
    class CEStateMachine;
    class CECommand;
    std::shared_ptr<CEStateMachine> stm;


    /* Callbacks */
    std::function<void(int)> move_left_cb;
    std::function<void(int)> move_right_cb;
    std::function<void(int)> move_up_cb;
    std::function<void(int)> move_down_cb;

    std::function<void(void)> invalid_command_cb;
};


#endif

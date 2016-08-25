#include "CommandEngine.h"



class CommandEngine::CEStateMachine :
    public std::enable_shared_from_this<CommandEngine::CEStateMachine>
{
public:
    static std::shared_ptr<CEStateMachine> Create();
    
    void processInput(const Key &input);


private:
    CEStateMachine() = default;

    /* Used by STMStateRegister to save entered register */
    void registerAcquired(char reg);

    /* Used by STMStateRepetition to save entered repetition
     * Repetitions are multiplied together */
    void repetitionAcquired(unsigned long long repetition);

    /* Called upon entering Init State, will reset saved command */
    void initStateEntered();



    // Command composition 
    unsigned long long repetition;
    char               reg;
    // Command ?
    

    // State
    class STMState;
    std::unique_ptr<STMState> state;

    void transition(const std::unique_ptr<STMState> new_state);



    using STM_weak_ptr = std::weak_ptr<CEStateMachine>;
    class STMState
    {
    public:
        STMState(const STM_weak_ptr &stm);
        virtual void processInput(const Key &input) = 0;
    protected:
        STM_weak_ptr stm;
    };

    class STMStateInit : public STMState
    {
    public:
        STMStateInit(const STM_weak_ptr &stm);
        STMStateInit(const STM_weak_ptr &stm, const Key &input);
        virtual void processInput(const Key &input);
    };

    class STMStateRegister : public STMState
    {
    public:
        STMStateRegister(const STM_weak_ptr &stm);
        STMStateRegister(const STM_weak_ptr &stm, const Key &input);
        virtual void processInput(const Key &input);
    private:
        bool isValidRegister(const Key &input) const;
        char reg{-1};
        bool received_leading_quote{false};
        bool capture_done{false};
    };

    class STMStateRepetition : public STMState
    {
    public:
        STMStateRepetition(const STM_weak_ptr &stm);
        STMStateRepetition(const STM_weak_ptr &stm, const Key &input);
        virtual void processInput(const Key &input);
    private:
        unsigned long long repetition;
    };

    class STMStateCommand : public STMState
    {
    public:
        STMStateCommand(const STM_weak_ptr &stm);
        STMStateCommand(const STM_weak_ptr &stm, const Key &input);
        virtual void processInput(const Key &input);
    private:
        //CECommand command;
    };

    class STMStateInvalid : public STMState
    {
    public:
        STMStateInvalid(const STM_weak_ptr &stm);
        STMStateInvalid(const STM_weak_ptr &stm, const Key &input);
        virtual void processInput(const Key &input);
    };

    class STMStateCommandComplete : public STMState
    {
    public:
        STMStateCommandComplete(const STM_weak_ptr &stm);
        STMStateCommandComplete(const STM_weak_ptr &stm, const Key &input);
        virtual void processInput(const Key &input);
    };
};

void CommandEngine::processInput(const Key &input)
{
}



/* Register basic arrow movements: up, down, left, and right
 * the callback function should accept single int parameter indicating
 * the repetition count of the movement */
void CommandEngine::registerLeftMove(std::function<void(int)> &f)
{
    move_left_cb = f;
}
void CommandEngine::registerRightMove(std::function<void(int)> &f)
{
    move_right_cb = f;
}
void CommandEngine::registerUpMove(std::function<void(int)> &f)
{
    move_up_cb = f;
}
void CommandEngine::registerDownMove(std::function<void(int)> &f)
{
    move_down_cb = f;
}

/* Whenever an invalid key is entered for the current state, this CB is
 * invoked, key processing will begin fresh with next key */
void CommandEngine::registerInvalidCommand(std::function<void(void)> &f)
{
    invalid_command_cb = f;
}



std::shared_ptr<CommandEngine::CEStateMachine>
CommandEngine::CEStateMachine::Create()
{
    std::shared_ptr<CEStateMachine> ptr(new CEStateMachine);

    ptr->transition(std::unique_ptr<STMState>(new STMStateInit(ptr)));

    return ptr;
}

void CommandEngine::CEStateMachine::processInput(const Key &input)
{
    state->processInput(input);

    if(dynamic_cast<STMStateInvalid*>(state.get()))
    {
        //invalid_command_cb();
    }
    else if(dynamic_cast<STMStateCommandComplete*>(state.get()))
    {
        //Dispatch callbacks
    }
}

/* Used by STMStateRegister to save entered register */
void CommandEngine::CEStateMachine::registerAcquired(char reg)
{
    this->reg = reg;
}

/* Used by STMStateRepetition to save entered repetition
 * Repetitions are multiplied together */
void CommandEngine::CEStateMachine::repetitionAcquired(unsigned long long repetition)
{
    this->repetition *= repetition;
}

/* Called upon entering Init State, will reset saved command */
void CommandEngine::CEStateMachine::initStateEntered()
{
    reg = '"';
    repetition = 1;
}

CommandEngine::CEStateMachine::STMState::STMState(const STM_weak_ptr &stm) :
    stm(stm)
{
}

CommandEngine::CEStateMachine::STMStateInit::STMStateInit(const STM_weak_ptr &stm) :
    STMState(stm)
{
}

CommandEngine::CEStateMachine::STMStateInit::STMStateInit(const STM_weak_ptr &stm, const Key &input) :
    STMState(stm)
{
    processInput(input);
}

void CommandEngine::CEStateMachine::STMStateInit::processInput(const Key &input)
{
    if(auto stm_ptr = stm.lock())
    {
        if(input.isValid() && input.isRegularKey())
        {
            if(input.getChar() == '"')
                stm_ptr->transition(std::make_unique<STMStateRegister>(stm, input));
            if(input.getChar() >= '1' && input.getChar() <= '9')
                stm_ptr->transition(std::make_unique<STMStateRepetition>(stm, input));
            else
                stm_ptr->transition(std::make_unique<STMStateCommand>(stm, input));
        }
    }
}

CommandEngine::CEStateMachine::STMStateRegister::STMStateRegister(const STM_weak_ptr &stm) :
    STMState(stm)
{
}

CommandEngine::CEStateMachine::STMStateRegister::STMStateRegister(const STM_weak_ptr &stm, const Key &input) :
    STMState(stm)
{
    processInput(input);
}

void CommandEngine::CEStateMachine::STMStateRegister::processInput(const Key &input)
{
    if(!received_leading_quote && input.getChar() == '"')
    {
        received_leading_quote = true;
    }
    else if(auto stm_ptr = stm.lock())
    {
        if(!capture_done)
        {
            if(isValidRegister(input))
            {
                reg = input.getChar();
                stm_ptr->registerAcquired(reg);
                capture_done = true;
            }
            else
                stm_ptr->transition(std::make_unique<STMStateInvalid>(stm, input));
        }
        else
        {
            if(input.isNumeric())
                stm_ptr->transition(std::make_unique<STMStateRepetition>(stm, input));
            else
                stm_ptr->transition(std::make_unique<STMStateCommand>(stm, input));
        }
    }
}

bool CommandEngine::CEStateMachine::STMStateRegister::isValidRegister(const Key &input) const
{
    if(!input.isValid() || !input.isRegularKey())
        return false;

    char value = input.getChar();

    if((value >= '0' && value <= '9') ||
       (value >= 'A' && value <= 'Z') ||
       (value >= 'a' && value <= 'z'))
        return true;

    switch(value)
    {
    case '"':
    case '-':
    case ':':
    case '.':
    case '%':
    case '#':
    case '=':
    case '*':
    case '+':
    case '~':
    case '_':
    case '/':
        return true;
    }
    return false;
}

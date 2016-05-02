#pragma once
#include <string>

class Dice
{
public:
    //CONSTRUCTORS//////////////////////////////////////////////////////////////////////////
    Dice(const std::string& diceString);
    //Dice(const char* diceString);
    Dice(unsigned int numDice, unsigned int numSides);
    ~Dice() {};

    //FUNCTIONS//////////////////////////////////////////////////////////////////////////
    unsigned int Roll() const;
    unsigned int Roll(unsigned int numRolls) const;

private:
    unsigned int m_numDice;
    unsigned int m_numSides;
};
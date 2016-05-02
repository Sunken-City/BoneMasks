#include "Engine/Math/Dice.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/MathUtils.hpp"

//-----------------------------------------------------------------------------------
Dice::Dice(unsigned int numDice, unsigned int numSides)
    : m_numSides(numSides)
    , m_numDice(numDice)
{

}

//-----------------------------------------------------------------------------------
Dice::Dice(const std::string& diceString)
{
    std::vector<std::string>* stringPieces = SplitString(diceString, "d");
    m_numDice = stoul(stringPieces->at(0));
    m_numSides = stoul(stringPieces->at(1));
    delete stringPieces;
}

//-----------------------------------------------------------------------------------
unsigned int Dice::Roll() const 
{
    unsigned int diceRollResult = 0;
    for (unsigned int i = 0; i < m_numDice; ++i)
    {
        diceRollResult += MathUtils::GetRandom(1, m_numSides);
    }
    return diceRollResult;
}

//-----------------------------------------------------------------------------------
unsigned int Dice::Roll(unsigned int numRolls) const
{
    unsigned int diceRollResult = 0;
    for (unsigned int i = 0; i < numRolls; ++i)
    {
        diceRollResult += Roll();
    }
    return diceRollResult;
}

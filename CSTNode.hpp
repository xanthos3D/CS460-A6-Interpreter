#ifndef CS460_A2_TOKENIZER_CSTNODE_HPP
#define CS460_A2_TOKENIZER_CSTNODE_HPP
#include <iostream>
#include <fstream>
#include <string>
#include "Token.hpp"



enum InstructionType {
    PUSH,          // Push a literal onto the stack
    LOAD,          // Load a variable’s value onto the stack
    STORE,         // Store the top of the stack into a variable
    ADD, SUB, MUL, DIV,  // Arithmetic operations
    JUMP,          // Unconditional jump
    JUMP_IF_FALSE, // Conditional jump if top of stack is false
    RETURN,        // Return from a function
    PRINT          // Print values
};

// Instruction structure
struct Instruction {
    InstructionType type;
    int intOperand;               // Optional integer operand (for literals or jump targets)
    std::string strOperand;       // Optional string operand (for variable names or format strings)

    // Constructor for instructions without operands
    // Example: Instruction(SUB)
    Instruction(InstructionType type) : type(type), intOperand(0), strOperand("") {}

    // Constructor for instructions with an integer operand
    //Instruction(PUSH, 1)
    Instruction(InstructionType type, int operand) : type(type), intOperand(operand), strOperand("") {}

    // Constructor for instructions with a string operand
    //Instruction(PRINT, "result")
    Instruction(InstructionType type, const std::string &operand) : type(type), intOperand(0), strOperand(operand) {}
};

/** **************************************************************************************
CstNode class


 *****************************************************************************************/
class CSTNode {
public:
    CSTNode(const Token& token) : token(token), _leftChild(nullptr), _rightSibling(nullptr),
                child(false),  _address(0){}

    CSTNode* getLeft() { return _leftChild;}
    CSTNode* getRight() { return _rightSibling;}
    void leftChild( CSTNode *newLeftChild ) { _leftChild = newLeftChild; }
    void rightSibling(CSTNode *newRightSibling) { _rightSibling = newRightSibling; }
    Token getToken() { return token; }
    void setChild(bool temp)   { child = temp; }        // this is to know when we are in a new statemnt
    // and our tree can refelct when to drop a level
    bool isChild() { return child; }


    //For intepreter
    int setAddress(int address) { return _address = address; };
    bool isFunction(){ return _isFunction; };
    void setisFunction() { _isFunction = true; }
    bool isMain(){ return _isMain; }
    void setIsMain() { _isFunction = true; } ;
    void setFunctionName(std::string functionName) { _functionName = functionName; }
    std::string getFunctionName() { return _functionName;  }



private:
    Token token;
    bool _isMain;
    bool _isFunction;
    bool child = false;
    CSTNode *_leftChild, *_rightSibling;
    int _address;
    std::string _functionName = "";
};



#endif //CS460_A2_TOKENIZER_CSTNODE_H

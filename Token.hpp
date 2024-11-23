#include <string>

#ifndef PROJECT1_PHASE2_TOKEN_HPP
#define PROJECT1_PHASE2_TOKEN_HPP

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

//Instruction structure
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

class Token {
public:
    // token recieves a line number ans position
    Token(int, int);

    // simple getter functions to tell if a token is a given type
    // note may want to have a switch statement that returns the type of token and store the token type as a string?
    bool &isIdentifier();
    bool &isDoubleQuote();
    bool &isSingleQuote();
    bool &isSemicolon();
    bool &isComma();
    bool &isEOF();

    bool &isLParen();
    bool &isRParen();
    bool &isLBrace();
    bool &isRBrace();
    bool &isLBracket();
    bool &isRBracket();

    bool &isAssignmentOperator();
    bool &isModulo();
    bool &isPlus();
    bool &isMinus();
    bool &isAsterisk();
    bool &isDivide();
    bool &isCarot();

    bool &isInt();
    bool &isDouble();
    bool &isChar();
    bool &isEscChar();
    bool &isString();

    bool &isBoolE();
    bool &isBoolNot();
    bool &isBoolNE();
    bool &isBoolGT();
    bool &isBoolLT();
    bool &isBoolGTE();
    bool &isBoolLTE();
    bool &isBoolAnd();
    bool &isBoolOr();
    bool &isBoolTrue();
    bool &isBoolFalse();

    bool &isImportant();
    bool &isFunctionName();

    //simple setter functions to set a token to a given type.
    void setEndOfFile();
    void setIdentifier(std::string);
    void setDoubleQuote();
    void setSingleQuote();
    void setSemicolon();
    void setComma();
    void setEOF();

    void setLParen();
    void setRParen();
    void setLBrace();
    void setRBrace();
    void setLBracket();
    void setRBracket();

    void setAssignmentOperator();
    void setModulo();
    void setPlus();
    void setMinus();
    void setAsterisk();
    void setDivide();
    void setCarot();

    void setInt(std::string);
    void setDouble(std::string);
    void setChar(std::string);
    void setEscChar(std::string);
    void setString(std::string);

    void setImportant();
    void setIsFuncName();


    void setBoolE();
    void setBoolNot();
    void setBoolNE();
    void setBoolGT();
    void setBoolLT();
    void setBoolGTE();
    void setBoolLTE();
    void setBoolAnd();
    void setBoolOr();
    void setBoolTrue();
    void setBoolFalse();



    //sets the tokenstring
    void setTokenString(std::string);

    //if a token has more complex information then store it in the token string
    std::string getTokenString();
    int getLineNum() { return _lineNumber; }
    int getCharPos() { return _charPos; }

    //useful tester function so we can print the tokens and there type.
    std::string print();

    //For intepreter we have some special setters 
    void setIsMain(){ _isMain = true; }
    bool getIsMain() { return _isMain; }
    int setAddress(int address) { return _address = address; };
    bool isFunction(){ return _isFunction; };
    void setisFunction() { _isFunction = true; }
    bool isMain(){ return _isMain; }
    void setFunctionName(std::string functionName) { _functionName = functionName; }
    std::string getFunctionName() { return _functionName;  }

private:

    bool _isFunction = false;
    bool child = false;
    int _address;
    std::string _functionName = "";

    //misc idenfitier types
    bool _identifier, _doubleQuote,_singleQuote, _semicolon,_comma, _eof, _isImportant, _isFuncName,
    //braces and brackets
    _LParen, _RParen, _LBrace, _RBrace,_LBracket, _RBracket,
    //opperators
    _assignmentOperator, _modulo, _plus, _minus, _asterisk, _divide, _caret,
    //data types
    _int,_double, _char, _string, _escChar,
    //boolean opperators
    _boolE,_boolNE, _boolGT, _boolLT, _boolGTE, _boolLTE, _boolAnd, _boolOr, _boolNot, _boolTrue,  _boolFalse;


    std::string _tokenString;
    int _lineNumber, _charPos;

    bool _isMain = false;
};

#endif //PROJECT1_PHASE2_TOKEN_HPP

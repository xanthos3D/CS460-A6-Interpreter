#include <iostream>
#include "Parser.hpp"
#include "CST.hpp"
#include "CSTNode.hpp"
#include "stack"
#include "SymbolTable.h"
#include <tuple>


/** **************************************************************************************
parse function which creates our CST
@pre:vector of tokens from declaration
@post: returns a concrete syntax tree.
 *****************************************************************************************/
CST* Parser::parse() {

    //calls our over arching function to recurse through our token vector
    program();
    return cst;
}

/** **************************************************************************************
Checks program meets BNF requirements
@pre:uses the token vector in our parse data object.
@post: recusively calls one of our functions based on the given token
 *****************************************************************************************/
void Parser::program() {

    while(!tokenVector[index].isEOF()){

        //std::cout<<"+++block start token++++++++++++++++++++++++++++++++++++++++++ "<<std::endl;
        //tokenVector[index].print();
        //std::cout<<"+++block start token++++++++++++++++++++++++++++++++++++++++++ "<<std::endl;
        if (tokenVector[index].getTokenString() == "function") {
            //std::cout<<" found function declaration" <<std::endl;
            inDeclaration = true;
            function_declaration();
            inDeclaration = false;
        } else if (tokenVector[index].getTokenString() == "procedure" && tokenVector[index + 1].getTokenString() != "main") {
            //std::cout<<" found  procedure declaration " <<std::endl;
            procedure_declaration();
        } else if ( datatype_specifier() ) {
            //std::cout<<" found  declaration statement " <<std::endl;
            int temp = scope;
            scope = 0;
            inDeclaration = true;
            declaration_statement();
            inDeclaration = false;
            scope = temp;
        } else if (tokenVector[index].getTokenString() == "procedure" && tokenVector[index + 1].getTokenString() == "main") { //needs  to be fixed to distinguish between procedure and procedure main
            //std::cout<<" found main procedure" <<std::endl;
            main_procedure();
        } else{
            //something wrong
            //std::cout<<"looked for declaration of function or procedure, but none found?"<<std::endl;
            std::cout<<"offending token:"<<std::endl;
            tokenVector[index].print();
            throw;
        }
    }

    //need to add our function params as param lists at the end of our parsing whem making the Symbol table.
    //nested loops which push the param lists onto the symboltable proper.
    std::cout<< "adding params to symbol table"<<std::endl;
    for(int i=0; i < paramLists.size(); i++){
        for(int j= 0; j < paramLists[i].size(); j++){
            std::cout<< "found param: " << paramLists[i][j].identifier_type <<std::endl;

            symbol_table_list.insertSymbol(paramLists[i][j], "");
        }
    }

}

/** **************************************************************************************
Checks main-procedure meets BNF requirements
@pre:uses the token vector in our parse data object.
@post:parses through the function headers and main procedures.
 *****************************************************************************************/
void Parser::main_procedure(){
    scope++;
    //SymbolTable new_symbol_table;
    new_symbol_table.scope = scope;
    new_symbol_table.identifier_type = tokenVector[index].getTokenString();
    expect("procedure");

    SymbolLocation = "main";
    new_symbol_table.identifier_name = tokenVector[index].getTokenString();
    expect("main");
    std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;
    symbol_table_list.insertSymbol(new_symbol_table,"main");


    //clear the temp symbol table element in our parser class.
    SymbolTable empty_symbol_table;
    new_symbol_table = empty_symbol_table;

    expect("(");
    expect("void");
    expect(")");
    block_statement();
}

/** **************************************************************************************
Checks function_declaration meets BNF requirements
@pre:is called when we find a function declaration
@post: handles generation of cst function block
 *****************************************************************************************/
void Parser::function_declaration(){
    inFunction = true;
    scope++;
    //SymbolTable new_symbol_table;
    new_symbol_table.scope = scope;
    //so when we find a function we expect the token to be a identifier with the string function
    new_symbol_table.identifier_type = tokenVector[index].getTokenString();
    expect("function");

    //then we expect some data type specifier, and if it is then
    if ( datatype_specifier() ) {
        //get that datatype name and call our expect function
        new_symbol_table.datatype = tokenVector[index].getTokenString();
        expect(tokenVector[index].getTokenString());
    }

    //then we expect an identifier/ function name
    if (tokenVector[index].isIdentifier() ) {
        if (datatype_specifier() || reserved_word()){
            throw std::runtime_error("Syntax error on line " + std::to_string(tokenVector[index].getLineNum()) + ": reserved word \"" +
                                     tokenVector[index].getTokenString() + "\" cannot be used for the name of a function.");
        }
        tokenVector[index].setIsFuncName();
        new_symbol_table.identifier_name = tokenVector[index].getTokenString();
        tempFunctionName = tokenVector[index].getTokenString();

        //new set the symbollocation here
        SymbolLocation = tokenVector[index].getTokenString();

        expect(tokenVector[index].getTokenString());
    }

    //tokenVector[index].setIfFuncName();
    // then a open paren
    expect( "(");
    //may need a loop to handle, multiple instances of parameters being declared.
    //if we see the keyword void, in our current token.
    if (tokenVector[index].getTokenString() == "void") {
        //expect the void token.
        expect("void");
    } else {
        // add the new symbol to our table
        std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;
        symbol_table_list.insertSymbol( new_symbol_table, SymbolLocation);
        //clear the temp symbol table element in our parser class.
        SymbolTable empty_symbol_table;
        new_symbol_table = empty_symbol_table;
        //otherwise we call our paramlist, until we reach a )
        parameter_list();
    }
    //now that we have eaten all our params and put them in paramlists
    paramLists.push_back(tempParamList);

    // after the params are added we want to clear our temp vector storing our params.
    tempParamList.clear();
    //tokenVector[index].setIfFuncName();
    expect( ")");
    expect( "{" );
    // calls compound statement to start the interior of our function block.
    compound_statement();
    expect( "}" );
    inFunction = false;

}

/** **************************************************************************************
Checks procedure_declaration meets BNF requirements
@pre:
@post:
 *****************************************************************************************/
void Parser::procedure_declaration(){
//    SymbolTable empty_symbol_table;
//    new_symbol_table = empty_symbol_table;
    scope++;
    //SymbolTable new_symbol_table;
    new_symbol_table.scope = scope;

    new_symbol_table.identifier_type = tokenVector[index].getTokenString();
    expect( "procedure");
    if ( tokenVector[index].isIdentifier() ) {
        new_symbol_table.identifier_name = tokenVector[index].getTokenString();
        tempFunctionName = tokenVector[index].getTokenString();
        // add the new symbol to our table
        std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;
        symbol_table_list.insertSymbol( new_symbol_table, SymbolLocation);
        //clear the temp symbol table element in our parser class.
//        //SymbolTable empty_symbol_table;
//        new_symbol_table = empty_symbol_table;
        SymbolLocation = tokenVector[index].getTokenString();
        expect(tokenVector[index].getTokenString());
    }
    expect( "(" );
    if (tokenVector[index].getTokenString() == "void") {
        expect("void");
    } else {

        parameter_list();
        //now that we have eaten all our params and put them in paramlists
        paramLists.push_back(tempParamList);

        // after the params are added we want to clear our temp vector storing our params.
        tempParamList.clear();
    }
    // add the new symbol to our table
//    std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;
//    symbol_table_list.insertSymbol( new_symbol_table );
    //clear the temp symbol table element in our parser class.
    SymbolTable empty_symbol_table;
    new_symbol_table = empty_symbol_table;
    expect( ")" );
    expect( "{" );
    compound_statement();
    expect( "}" );
}

/** **************************************************************************************
checks that the parameter lsit  follows BNF rules
@pre: is called when we find parameter for a function
@post:generates cst for parameters
 *****************************************************************************************/
void Parser::parameter_list(){
    inParamList = true;
    //used to revert to incoming value of inDeclaration
    bool revertTo = inDeclaration;
    //if we find a data type
    if ( datatype_specifier() ){
        inDeclaration = true;
        //set data type of paramlist symboltable to this data type.
        new_symbol_table.datatype = tokenVector[index].getTokenString();
        //call expect with that data type
        expect(tokenVector[index].getTokenString() );
    }

    //define the parameters variable name.
    new_symbol_table.identifier_type = tokenVector[index].getTokenString();

    //then expect variable name
    identifier();

    //then an array, can be valid syntax for a param list. so if we find a bracket
    if(tokenVector[index].isLBracket()){
        //call our function to handle an array [i]

        identifier_and_identifier_array_list();
    }

    // then we check to see if a comma is present
    if ( tokenVector[index].isComma()) {
        //if it is then add it
        expect(",");

        //set scope of variable
        new_symbol_table.scope = scope;

        //set the paramfunction identifier for param symbol.
        new_symbol_table.identifier_name = tempFunctionName;

        //set the paramfunction identifier for param symbol.
        new_symbol_table.isParamList = true;

        //save symbol parameter to our vector
        tempParamList.push_back(new_symbol_table);

        //clear the vector
        SymbolTable empty_symbol_table;
        new_symbol_table = empty_symbol_table;
        // and recuse until we are out of parameters to add.
        parameter_list();
    }else if(tokenVector[index].isRParen()){
        //set scope of variable
        new_symbol_table.scope = scope;

        //set the paramfunction identifier for param symbol.
        new_symbol_table.identifier_name = tempFunctionName;

        //set the paramfunction identifier for param symbol.
        new_symbol_table.isParamList = true;

        //save symbol parameter to our vector
        tempParamList.push_back(new_symbol_table);

        //clear the vector
        SymbolTable empty_symbol_table;
        new_symbol_table = empty_symbol_table;
        inParamList = false;
    }

    inDeclaration = revertTo;
}

/** **************************************************************************************
checks that the follwoing statement is a block statement that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::block_statement(){
    expect( "{");
    if( !tokenVector[index].isRParen()) {
        compound_statement();
    }
    expect( "}");
}

/** **************************************************************************************
checks that the follwoing statement is a compound statementthat follows BNF rules
@pre: called at the beginning of a scope block
@post:recurses through the scope of that block
 *****************************************************************************************/
void Parser::compound_statement() {

    //unless we find a closing brace
    if ( !tokenVector[index].isRBrace() ) {
        //check for statements and recurse this function.
        statement();
        compound_statement();
    }
}

/** **************************************************************************************
checks that the follwoing statement makes sure its a statemtn defined in BNF
@pre:is called to find a statement
@post: recurses though tokens to create a statement.
 *****************************************************************************************/
void Parser::statement(){

    //make a temp token?
    Token token = tokenVector[index];

    //std::cout<<"statement called test: ("<< token.getTokenString()<<")"<<std::endl;

    //cases to recurse through a variety of different statements.
    if (token.getTokenString() == "return"){
        std::cout<<"return found"<<std::endl;
        return_statement();
    }else if ( token.getTokenString() == "if"){
        std::cout<<"if found"<<std::endl;
        selection_statement();
    }else if (token.getTokenString() == "else"){
        std::cout<<"else found"<<std::endl;
        selection_statement();
    }else if (token.getTokenString() == "printf"){
        std::cout<<"printf found"<<std::endl;
        printf_statement();
    }else if (token.getTokenString() == "for" || token.getTokenString() == "while"){
        std::cout<<"for/while found"<<std::endl;
        iteration_statement();
    } else if ((token.isIdentifier() && tokenVector[index + 1].isAssignmentOperator()) ||
               (token.isIdentifier() && tokenVector[index + 1].isLBracket()) ||
               (token.isIdentifier() && tokenVector[index + 1].isLParen())){
        if (tokenVector[index + 1].isAssignmentOperator() || token.isIdentifier() && tokenVector[index + 1].isLBracket()) {
            std::cout << "assignment found" << std::endl;
            assignment_statement();
        }else if (tokenVector[index + 1].isLParen()){
            //std::cout << "user defined func found" << std::endl;
            user_defined_function();
            expect(";");
        }

        //case which handles variable declarizations and assignments.
    } else if ( datatype_specifier() || token.isIdentifier() ) {
        std::cout<<"declaration found: "<< token.getTokenString()<<std::endl;
        if(datatype_specifier()){
            token.print();
            inDeclaration = true;
        }
        declaration_statement();
        inDeclaration = false;
    }else{
        std::cout<<"error?"<<std::endl;
    }
}

/** **************************************************************************************
checks that the follwoing statement is a return statement that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::return_statement(){
    expect("return");
    if (tokenVector[index].isSingleQuote() || tokenVector[index].isDoubleQuote()) {
        expect(tokenVector[index].getTokenString());
    }
    else {
        expression();
    }
    expect(";");
}

/** **************************************************************************************
Checks declaration_statement meets BNF requirements
if scope is 0 and name is already taken then error for being defined globally
if scope is greater than 0 and name is already taken then error for being defined locally
@pre:
@post:
 *****************************************************************************************/
void Parser::declaration_statement(){
    std::cout<<"in declaration_statement"<<std::endl;
    //if we recieve a specifier(data type)
    //SymbolTable new_symbol_table;
    new_symbol_table.scope = scope;
    if ( datatype_specifier() ){
        new_symbol_table.datatype = tokenVector[index].getTokenString();
        new_symbol_table.identifier_type = "datatype";

        expect( tokenVector[index].getTokenString());

        //then we expect an identifier after it.
        if (tokenVector[index].isIdentifier()) {
            new_symbol_table.identifier_name = tokenVector[index].getTokenString();
            std::cout<<"found identifier after a data specifier."<<std::endl;
            identifier();
            //double check this
            //expect(tokenVector[index].getTokenString());
        } else {
            throw;
        }
    }

    //if we see a left bracket we are declaring an array
    if ( tokenVector[index].isLBracket()){
        identifier_and_identifier_array_list();
        if (inDeclaration) {
            new_symbol_table.DATATYPE_IS_ARRAY = "yes";
        }
    }

    // add the symbol but dont clear the temp data, as we may have more to add if there is a comma, but dont want to clear the values being declared.
    if(inDeclaration == true){
        std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;
        symbol_table_list.insertSymbol(new_symbol_table,SymbolLocation);
    }else{
        std::cout<<"found symbol but not inserting: "<<new_symbol_table.identifier_name <<std::endl;
    }

    //if we see a comma after ward, that means that multiple things are being declared
    if ( tokenVector[index].isComma()){
        expect(",");
        std::cout<<"found a comma now going into identifier_and_identifier_array_list."<<std::endl;
        identifier_and_identifier_array_list();
    }
    //expect statement to end with a semicolon.
    if(tokenVector[index].isRParen()){
        return;
    }else{

        //clear the vectorafter we are done with declaration
        SymbolTable empty_symbol_table;
        new_symbol_table = empty_symbol_table;
        inDeclaration = false;
        expect(";");
    }
}

/** **************************************************************************************
checks that the follwoing function is a user_defined_function  that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::user_defined_function(){
    if (tokenVector[index].isIdentifier()) {
        expect( tokenVector[index].getTokenString() );
    }
    tokenVector[index].setImportant();
    expect("(");
    if (tokenVector[index].isIdentifier()) {
        identifier_and_identifier_array_list();
    } else{
        expression();
    }
    tokenVector[index].setImportant();
    expect(")");

}

/** **************************************************************************************
checks that the follwoing function is a getchar function that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::getchar_function(){
    expect("getchar");
    expect("(");
    if (tokenVector[index].isIdentifier()) {
        expect( tokenVector[index].getTokenString() );
    }
    expect(")");

}

/** **************************************************************************************
checks that the follwoing statement is a printf statement that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::printf_statement(){

    expect("printf");
    expect("(");
    //after the first double quote
    if (tokenVector[index].isDoubleQuote()){
        //eat the string
        double_quoted_string();
        //if there is a comma, variables may be involved, so acount for those here
        if ( tokenVector[index].isComma() ){
            expect(",");
            identifier_and_identifier_array_list();
        }
    }else if (tokenVector[index].isSingleQuote()){
        single_quoted_string();
        //if there is a comma, variables may be involved, so acount for those here
        if ( tokenVector[index].isComma() ){
            expect(",");
            identifier_and_identifier_array_list();
        }
    }
    expect(")");
    expect(";");

}

/** **************************************************************************************
checks that the follwoing statement is a assignment statement that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::assignment_statement(){
    inImportantExp = true;
    expect( tokenVector[index].getTokenString() );
    if (tokenVector[index].isLBracket()){
        tokenVector[index].setImportant();
        expect("[");
        expect( tokenVector[index].getTokenString() );
        tokenVector[index].setImportant();
        expect("]");
    }
    expect("=");

    if (tokenVector[index].isSingleQuote()){
        single_quoted_string();
    }else if (tokenVector[index].isDoubleQuote()) {
        double_quoted_string();
    }else {
        expression();
    }
    expect(";");
    inImportantExp = false;
}

/** **************************************************************************************
checks that the follwoing statement is an iteration statement that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::iteration_statement() {

    //if identifier is a for loop then
    if (tokenVector[index].getTokenString() == "for") {

        //set for loop to true, so that we handle semicolons correctly within for statement
        inForLoop = true;
        expect(tokenVector[index].getTokenString());
        expect("(");
        //std::cout<<"entering initilization statement for for loop"<<std::endl;
        initialization_statement();
        if (tokenVector[index].isSemicolon()) {
            expect(tokenVector[index].getTokenString());
        }
        //std::cout<<"entering boolean statement for for loop"<<std::endl;
        boolean_expression();
        if (tokenVector[index].isSemicolon()) {
            expect(tokenVector[index].getTokenString());
        }
        //std::cout<<"entering increment expression statement for for loop"<<std::endl;
        expression();
        expect(")");
        inForLoop = false;
        if (tokenVector[index].isLBrace()) {
            block_statement();
        } else {
            statement();
        }
        //set for loop to false, so we handle semicolons normally after this.
        //  inForLoop = false;
    } else if (tokenVector[index].getTokenString() == "while" ){
        expect(tokenVector[index].getTokenString());
        expect("(");
        boolean_expression();
        expect(")");
        if (tokenVector[index].isLBrace()) {
            block_statement();
        } else {
            statement();
        }
    }
}

/** **************************************************************************************
checks that the follwoing statement is a selection statement that follows BNF rules
@pre: when token starts a if
@post: generate if cst statement
 *****************************************************************************************/
void Parser::selection_statement(){
    inImportantExp = true;
    //first token should be a if statement.
    if (tokenVector[index].getTokenString() == "if"){
        expect(tokenVector[index].getTokenString());
    }

    //then a open paren
    expect("(");
    //bool expression
    boolean_expression();
    //closed paren
    expect(")");
    inImportantExp = false;
    // then a block statement
    if (tokenVector[index].isLBrace()) {
        //recurse througho ur block function.
        block_statement();
        if (tokenVector[index].getTokenString() == "else"){
            expect(tokenVector[index].getTokenString());
            if (tokenVector[index].isLBrace()) {
                block_statement();
            } else {
                statement();
            }
        }
    } else {
        statement();
        if (tokenVector[index].getTokenString() == "else "){
            expect(tokenVector[index].getTokenString());
            if (tokenVector[index].isLBrace()) {
                block_statement();
            } else {
                statement();
            }
        }
    }
}

/** **************************************************************************************
checks that the follwoing statement is a initialization expression that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::expression(){
    std::cout<<"in expression"<<std::endl;
    //if we find a token to be true or false boolean then
    if (tokenVector[index].isBoolTrue() || tokenVector[index].isBoolFalse() ) {
        //call our boolean expression code
        boolean_expression();
        //if we find a int
    } else if (tokenVector[index].isInt()) {
        //call numeric expression
        numerical_expression();
        //if param is integer then set it.
        //new_symbol_table.identifier_type =
        //if we find a identifier and a left paren
    } else if (tokenVector[index].isIdentifier() && ( ( tokenVector[index + 1].isLParen()) )) {

        //sets paramlist with given function name.
        //new_symbol_table.identifier_name = tokenVector[index].getTokenString();

        //built in function to search our symbol table to see if the function exists and extracts its data type for the param list.
        //new_symbol_table.datatype = symbol_table_list.findFunctionDataType(tokenVector[index].getTokenString());

        //std::cout<<"in expression, expecting function with paren"<<std::endl;
        //then we found a function and want to encase the next expression in parenthesis
        expect( tokenVector[index].getTokenString() );


        //set the symbol table struct to take a paramlist
        //new_symbol_table.isParamList = true;
        // set scope of paramlist, is it always one for function calls or is it dependant on other factors?
        //new_symbol_table.scope = 1;



        //note: how do we set the data type? has to match the function definition right? maybe search for it in our symbol table?
        tokenVector[index].setImportant();
        expect("(");
        //set new symbol table object
        //new_symbol_table.identifier_type = tokenVector[index].getTokenString();
        expression();
        tokenVector[index].setImportant();
        expect(")");
        //need to return symbol here for the function call.
        //symbol_table_list.insertSymbol( new_symbol_table );
        //clear the temp symbol table element in our parser class.
        //SymbolTable empty_symbol_table;
        //new_symbol_table = empty_symbol_table;

        return;
        //if we find a identifier and therei s a demicolon agter it
    }else if (tokenVector[index].isIdentifier() && ( ( tokenVector[index + 1].isSemicolon() ) )) {
        std::cout<<"in expression, expecting function with paren"<<std::endl;
        //expect the identifier and then return
        expect( tokenVector[index].getTokenString() );
        return;
        //if we find a identifier and a bracket after it.
    }else if (tokenVector[index].isIdentifier() && ( ( tokenVector[index + 1].isLBracket()))) {
        //then we found an array and expect it to close properly
        std::cout<<"in expression, expecting function with bracket so array"<<std::endl;
        //eat the identifier
        expect( tokenVector[index].getTokenString() );

        //then handle array with out declaration statement code.
        declaration_statement();
        return;
    }else if (tokenVector[index].isIdentifier()) {
        std::cout<<"in expression, expecting variable"<<std::endl;

        //if we see a assignment operator after a identifier
        if(tokenVector[index + 1].isAssignmentOperator()){
            //eat identifier and then assignment operator
            expect(tokenVector[index].getTokenString());
            expect("=");

            //then call our numerical expression function
            numerical_expression();

        }else if ( relational_expression() || relational_expression() ) {
            boolean_expression();
        }
        numerical_expression();
    }
    if ( tokenVector[index].isLParen() ) {
        expect("(");
        expression();
        expect(")");
    }
}


/** **************************************************************************************
checks that the follwoing statement is a initialization expression that follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::initialization_statement(){

    if (tokenVector[index].isIdentifier() && ( tokenVector[index + 1].isAssignmentOperator() ) ) {
        expect( tokenVector[index].getTokenString() );
        expect( tokenVector[index].getTokenString() );
        if  (tokenVector[index].isSingleQuote() ){
            single_quoted_string();
        } else if ( tokenVector[index].isDoubleQuote() ){
            double_quoted_string();
        } else {
            expression();
        }
    }
}







/** **************************************************************************************
checks that the follwoing expression is a boolean expression that follows BNF rules
@pre:recieves start to boolean expression
@post: creates cst expression for boolean
 *****************************************************************************************/
void Parser::boolean_expression(){
    std::cout<<"in boolean_expression"<<std::endl;
    // if we find keyword true or false
    if (tokenVector[index].isBoolTrue()){
        expect( "TRUE");

        // then check to see if a operator follows it.
        if(relational_expression()){
            //then another expresion should follow it
            boolean_expression();
        }
    }else if (tokenVector[index].isBoolFalse()){
        expect( "FALSE");

        // then check to see if a operator follows it.
        if(relational_expression()){
            //then another expresion should follow it
            boolean_expression();
        }

        //if we find a identifier
    }else if(tokenVector[index].isBoolNot()){

        //eat !
        expect("!");

        //recursively call our boolean expression
        boolean_expression();

        //if we find a open paren
    }else if(tokenVector[index].isIdentifier()){

        //could be a variable so eat up input
        expect(tokenVector[index].getTokenString());

        //could be an array refrence or a function call
        if(tokenVector[index].isLBracket() || tokenVector[index].isLParen()){
            identifier_and_identifier_array_list();
        }

        //after we figure out what we are working with then check if next symbol is a bool operator
        if(relational_expression()){
            //then another expresion should follow it
            boolean_expression();
            //it could also be a numerical operator which in that case
        }else if(numerical_operator()){
            //then call numericla expression after we eat the numerical operator.
            numerical_expression();
        }

        //if we find a open paren
    }else if(tokenVector[index].isDoubleQuote()){

        // note, do we need to acount for ' (char) ' as well?
        //eat ""
        expect("\"");
        //we expect a string after the first quote
        if(tokenVector[index].isString()){
            //eat string, then quote
            expect(tokenVector[index].getTokenString());
            expect("\"");
        }else{
            std::cout << "in boolean expression expected a string after a quote but recieved: " << tokenVector[index].getTokenString() << std::endl;
        }

        //recursively call our boolean expression
        boolean_expression();

        //if we find a open paren
    }else if (tokenVector[index].isLParen() ){
        //eat paren
        expect("(");

        if(tokenVector[index].isBoolNot()){
            expect("!");

            //recursively call our boolean expression
            boolean_expression();
            //if we find a open paren
        }

        // if we find a identifier
        if (tokenVector[index].isIdentifier()) {
            //could just receive an identifier by its self so recursive call so it's caught by our
            //case above where the identifier could be a array for function call.
            boolean_expression();

        }
        expect(")");

        if(relational_expression()){
            //then another expression should follow it
            boolean_expression();
            //it could also be a numerical operator which in that case
        }


    }else if (tokenVector[index].isSingleQuote() ){
        //eat paren
        single_quoted_string();

        boolean_expression();

    }else{

        //std::cout<<"in else case!"<<std::endl;
        numerical_expression();
        if (  relational_expression() ){
            expect( tokenVector[index].getTokenString() );
        }
        numerical_expression();

        //after we figure out what we are working with then check if next symbol is a operator
        if(relational_expression()){
            //then another expression should follow it
            boolean_expression();
        }
    }
    //std::cout<<"end boolean_expression"<<std::endl;
}



/** **************************************************************************************
checks that the follwoing expression is a numerical expression that follows BNF rules
@pre: is called when we find a neumeric expression for a bool operation
@post:
 *****************************************************************************************/
void Parser::numerical_expression() {
    std::cout<<"in numerical_expression"<<std::endl;

    bool eat = true;
    while(eat){

        //loop is empty, but the heavy lifting is done in the bool checks
        if(tokenVector[index].isLParen()){
            //std::cout<<"begin paren inside of numerical expression"<<std::endl;
            //eat up the parens
            expect("(");
            //we then need to recurse into a new numeric expression call.
            numerical_expression();
            //then we expect a close paren
            expect(")");

            //std::cout<<"end paren inside of numerical expression"<<std::endl;

        }else{
            eat = numerical_operand();
            //attempt to eat, but not an operand

            if(eat == false && tokenVector[index].isIdentifier()){
                //call expression function to make function call?
                expression();
            }


        }

        //then if there is a operator afterward, need to eat that before going to next loop
        eat = numerical_operator();

        if(relational_expression()){
            boolean_expression();
        }

    }


}


/** **************************************************************************************
checks if token is a relational_expression
@pre:
@post:
 *****************************************************************************************/
bool Parser::relational_expression(){
    std::cout<<"in relational_expression/boolean_operator"<<std::endl;
    if (tokenVector[index].isBoolE()){
        expect("==");
        return true;
    }else if (tokenVector[index].isBoolNE()){
        expect("!=");
        return true;
    }else if (tokenVector[index].isBoolGT()){
        expect(">");
        return true;
    }else if (tokenVector[index].isBoolLT()){
        expect("<");
        return true;
    }else if (tokenVector[index].isBoolGTE()){
        expect(">=");
        return true;
    }else if (tokenVector[index].isBoolLTE()){
        expect("<=");
        return true;
    }else if (tokenVector[index].isBoolOr()){
        expect("||");
        return true;
    }else if (tokenVector[index].isBoolAnd()){
        expect("&&");
        return true;
    }else{
        //std::cout<<"token is not currently a boolean operator"<<std::endl;
        return false;
    }
}





/** **************************************************************************************
checks if token is a boolean operator
@pre:
@post:
 *****************************************************************************************/
bool Parser::numerical_operator(){
    //std::cout<<"in numerical_operator"<<std::endl;
    if (tokenVector[index].isPlus()){
        expect("+");
        return true;
    }else if (tokenVector[index].isMinus()){
        expect("-");
        return true;
    }else if (tokenVector[index].isAsterisk()){
        expect("*");
        return true;
    }else if (tokenVector[index].isDivide()){
        expect("/");
        return true;
    }else if (tokenVector[index].isModulo()){
        expect("%");
        return true;
    }else if (tokenVector[index].isCarot()){
        expect("^");
        return true;
    }else{
        //std::cout<<"token is not currently a numerical operator"<<std::endl;
        return false;
    }
}

/** **************************************************************************************
checks if numerical_operand follows BNF
@pre:
@post:
 *****************************************************************************************/
bool Parser::numerical_operand(){
    //std::cout<<"in numerical_operand"<<std::endl;
    //if we find a int
    if (tokenVector[index].isIdentifier()) {
        if (tokenVector[index + 1].isLParen()) {
            user_defined_function();
        } else {
            expect( tokenVector[index].getTokenString() );
        }
        return true;
        // if we find a identifier
    } else if (tokenVector[index].isInt()){
        expect( tokenVector[index].getTokenString() );

        return true;
        //if we find a char
    }else if (tokenVector[index].getTokenString() == "getchar"){
        getchar_function();

        return true;
        //if we find a single quote
    }else if (tokenVector[index].isSingleQuote()){
        if(inImportantExp){
            tokenVector[index].setImportant();
        }
        expect("'");
        if(tokenVector[index].isChar()){
            expect( tokenVector[index].getTokenString() );
        }else if (tokenVector[index].isString()){
            expect( tokenVector[index].getTokenString());
        }else if (tokenVector[index].isEscChar()){
            expect( tokenVector[index].getTokenString() );
        }else{
            //ERROR ERROR ERROR ERROR ERROR ERROR
        }
        if(inImportantExp){
            tokenVector[index].setImportant();
        }
        expect("'");

        return true;
        //if we find a double quote
    }else if (tokenVector[index].isDoubleQuote()){
        if(inImportantExp){
            tokenVector[index].setImportant();
        }
        expect("\"");
        if(tokenVector[index].isChar()){
            expect( tokenVector[index].getTokenString() );
        }else if (tokenVector[index].isEscChar()){
            expect( tokenVector[index].getTokenString() );
        }else{
            //ERROR ERROR ERROR ERROR ERROR ERROR
        }
        if(inImportantExp){
            tokenVector[index].setImportant();
        }
        expect("\"");
        return true;
    } else {
        //ERROR ERROR ERROR ERROR ERROR ERROR ERROR
        return false;
    }
}






/** **************************************************************************************
returns if token is a datatype specifier
@pre:
@post:
 *****************************************************************************************/
bool Parser::datatype_specifier(){
    return (tokenVector[index].getTokenString() == "int" || tokenVector[index].getTokenString() == "char" ||
            tokenVector[index].getTokenString() == "bool");
}

/** **************************************************************************************
returns if token is a datatype specifier
@pre:
@post:
 *****************************************************************************************/
bool Parser::reserved_word(){
    return (tokenVector[index].getTokenString() == "if" || tokenVector[index].getTokenString() == "else" ||
            tokenVector[index].getTokenString() == "void" || tokenVector[index].getTokenString() == "main" ||
            tokenVector[index].getTokenString() == "procedure" || tokenVector[index].getTokenString() == "for" ||
            tokenVector[index].getTokenString() == "while" || tokenVector[index].getTokenString() == "printf" ||
            tokenVector[index].getTokenString() == "getchar" || tokenVector[index].getTokenString() == "return" ||
            tokenVector[index].getTokenString() == "function");
}





/** **************************************************************************************
checks if token is followed by identifier list or array lsit
@pre:
@post:
 *****************************************************************************************/
void Parser::identifier_and_identifier_array_list() {
    std::cout<<"in identifier_and_identifier_array_list"<<std::endl;
    if (tokenVector[index].isInt()){
        index -= 2;
    }

    //special case, if we end up finding a identifier here then
    if(tokenVector[index].isIdentifier()){
        std::cout<<"now eating identifier: "<<tokenVector[index].getTokenString()<<std::endl;

        std::cout<<"new_symbol_table.identifier_type: "<<new_symbol_table.identifier_type<<std::endl;

        //if there are multiple values being declared on one line this should cover it
        //ex int 1,j,k;
        new_symbol_table.identifier_name = tokenVector[index].getTokenString();
        if(inDeclaration == true){
            std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;
            symbol_table_list.insertSymbol(new_symbol_table,SymbolLocation);
        }else{
            std::cout<<"not in declaration so didnt add symbol: "<<new_symbol_table.identifier_name <<std::endl;
        }

        expect(tokenVector[index].getTokenString());
    }

    //after that identifier is there was one, or if we dont hav to worry about one
    //do we have a bracket, if so then we have a array, go to that function logic
    if ( tokenVector[index].isLBracket()) {
        if (inDeclaration) {
            new_symbol_table.DATATYPE_IS_ARRAY = "yes";
        }
        identifier_array_list();

        //otherwise if its a open paren then we have a function call
    }else if (tokenVector[index].isLParen()) {
        if (inImportantExp){
            tokenVector[index].setImportant();
        }
        expect("(");
        identifier_list();
        if (inImportantExp){
            tokenVector[index].setImportant();
        }
        expect(")");

        //if we see a comma by its self then call identifier list. mostelikely called after identifier list was finished with a function call or array[a]
    }else if (tokenVector[index].isComma()) {
        expect(",");
        identifier_list();
    } else if (tokenVector[index].isIdentifier() /*&& tokenVector[index + 1].isSemicolon()*/){
        identifier();

    }else{
        std::cout<<"flagging to find comma, esaping identifier_and_identifier_array_list: "<<std::endl;
        //throw std::runtime_error("Expected an identifier, but got '" + tokenVector[index].getTokenString() + "'");
    }

    //std::cout<<"end identifier_and_identifier_array_list"<<std::endl;
}





/** **************************************************************************************
checks identifier  list follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::identifier_list() {
    std::cout<<"in identifier list"<<std::endl;

    //if we find, a identifier, or a data type
    if(tokenVector[index].isIdentifier()){

        new_symbol_table.identifier_name = tokenVector[index].getTokenString();
        std::cout<<"inserting symbol: "<<new_symbol_table.identifier_name <<std::endl;

        if(inDeclaration == true){
            symbol_table_list.insertSymbol(new_symbol_table,SymbolLocation);
        }

        //eat that value
        expect(tokenVector[index].getTokenString());

        //then if we find a comma after that we recurse on this function
        if(tokenVector[index].isComma()){
            std::cout<<"recursively calling identifier list"<<std::endl;
            //eat the comma
            expect(",");
            //recurse to get the next identifier in our list
            identifier_list();
            //found a function call or a array in identifier list we call our identifier_and_identifier_array_list() function
        }else if(tokenVector[index].isLParen() || tokenVector[index].isLBracket()){
            identifier_and_identifier_array_list();
        }
    }else{
        //throw error, potentially called after we found a comma but no proper identifier or data type was found.
    }

}





/** **************************************************************************************
checks identifier array list follows BNF rules
@pre:
@post:
 *****************************************************************************************/
void Parser::identifier_array_list() {
    //std::cout<<" in identifier_array_list"<<std::endl;
    //std::cout<<"token is: "<<tokenVector[index].getTokenString()<<std::endl;

    if(inImportantExp){
        tokenVector[index].setImportant();
    }
    // ok so we found an array call, we expect to be at the open lbracket
    expect( "[" );

    if (tokenVector[index].getTokenString().find("-") != std::string::npos){
        throw std::runtime_error("Syntax error on line " + std::to_string(tokenVector[index].getLineNum()) + ": array declaration must be a positive integer.");
    }
        //then we expect either an int which we eat
    else if (tokenVector[index].isInt()) {
        if (inDeclaration) {
            new_symbol_table.datatype_array_size = tokenVector[index].getTokenString();
        }
        expect(tokenVector[index].getTokenString());
        //or an identifier that we eat
    }else if(tokenVector[index].isIdentifier()){
        if (inDeclaration) {
            new_symbol_table.datatype_array_size = tokenVector[index].getTokenString();
        }
        expect(tokenVector[index].getTokenString());
    }
    if(inImportantExp){
        tokenVector[index].setImportant();
    }
    //then we expect a closing bracket
    expect( "]" );

    //recursively call our array if there is an extra dimension to our array.
    if ( tokenVector[index].isComma() ) {
        if (inParamList){
            return;
        }else {
            identifier_array_list();
        }
    }
    //std::cout<<" ending identifier_array_list"<<std::endl;
}





/** **************************************************************************************
checks if token is identifier
@pre:
@post:
 *****************************************************************************************/
void Parser::identifier(){
    if (!tokenVector[index].isIdentifier()){
        throw std::runtime_error("Expected an identifier, but got '" + tokenVector[index].getTokenString() + "'");
    }
    if (datatype_specifier() || reserved_word()){
        throw std::runtime_error("Syntax error on line " + std::to_string(tokenVector[index].getLineNum()) + ": reserved word \"" +
                                 tokenVector[index].getTokenString() + "\" cannot be used for the name of a variable.");
    }
    //loop through takenNames to see if name has been used try to also determine if
    //defined locally or globally
    for(int i = 0; i < takenNames.size(); i++){
        if(takenNames[i].first == tokenVector[index].getTokenString() && takenNames[i].second == 0){
            std::cerr << "Error on line " << std::to_string(tokenVector[index].getLineNum())
                      << ": variable \"" << tokenVector[index].getTokenString() << "\" is already defined globally" << std::endl;
            exit(1);
        }
        else if(takenNames[i].first == tokenVector[index].getTokenString() && takenNames[i].second == scope){
            std::cerr << "Error on line " << std::to_string(tokenVector[index].getLineNum())
                      << ": variable \"" << tokenVector[index].getTokenString() << "\" is already defined locally" << std::endl;
            exit(1);
        }
    }
    //add name and scope to vector for later checks
    takenNames.push_back(std::make_pair(tokenVector[index].getTokenString(), scope));
    expect( tokenVector[index].getTokenString() );
}


/** **************************************************************************************
checks if single quoted string
@pre:
@post:
 *****************************************************************************************/
void Parser::single_quoted_string(){
    std::cout<<"in single quote"<<std::endl;
    if (!tokenVector[index].isSingleQuote()){
        throw std::runtime_error("Expected a single quote, but got '" + tokenVector[index].getTokenString() + "'");
    }
    if (inImportantExp) {
        tokenVector[index].setImportant();
    }
    expect( "\'");
    expect(tokenVector[index].getTokenString());
    if (inImportantExp) {
        tokenVector[index].setImportant();
    }
    expect("\'");
}

/** **************************************************************************************
checks if double quoted string
@pre:
@post:
 *****************************************************************************************/
void Parser::double_quoted_string(){
    if (!tokenVector[index].isDoubleQuote()){
        throw std::runtime_error("Expected a double quote, but got '" + tokenVector[index].getTokenString() + "'");
    }
    if (inImportantExp) {
        tokenVector[index].setImportant();
    }
    expect( "\"");
    expect(tokenVector[index].getTokenString());
    if (tokenVector[index].isEOF()){
        throw std::runtime_error("Syntax error on line " + std::to_string(tokenVector[index].getLineNum()) + ": unterminated string quote.");
    }
    if (inImportantExp) {
        tokenVector[index].setImportant();
    }
    expect("\"");
}

/** **************************************************************************************
If the token string matches expected_value then it adds it to the CST tree
@pre:
@post:
 *****************************************************************************************/
void Parser::expect(const std::string& expected_value) {
    //protective if to ensure we are within out vector of tokens.
    if (index < tokenVector.size()) {
        //make a token defined with the current token in our vector at our index position.
        Token token = tokenVector[index];
        //tokenVector[index].print();        //Use this to debug and know which token youre at
        if (token.getTokenString() != expected_value) {
            throw std::runtime_error("Expected '" + expected_value + "' but got: '" + tokenVector[index].getTokenString() + "' on line " + std::to_string(tokenVector[index].getLineNum()));
        }

        //if index is zero, then make new root
        if (index == 0){
            cst->setRoot(new CSTNode(tokenVector[index]));
            //if we find a left or right brace then make a right child of that token
        }else if (token.isLBrace()|| token.isRBrace() ){
            newStatement = true;
            cst->addChild( cst->getRoot(), token );
            //sibling for semicolon, but only if we arnt in a for loop
        }else if ( token.isSemicolon() && inForLoop == false){
            newStatement = true;
            cst->addSibling( cst->getRoot(), token );

            //sibling if we are in a for loop then we havent entered a new statement.
        }else if ( token.isSemicolon() && inForLoop == true){
            newStatement = false;
            cst->addSibling( cst->getRoot(), token );

        }else if (newStatement){
            cst->addChild( cst->getRoot(), token );
            newStatement = false;
        } else {
            cst->addSibling( cst->getRoot(), token ) ;
        }
        index++;
    } else {
        throw std::runtime_error("Unexpected end of input");
    }


}

void Parser::PrintSymbolTableLL( ){
    symbol_table_list.PrintSymbolTableList();
}

//converts cst to ast, then returns the ast in cst
CST* Parser::convertToAST(){
    cst->cstToAst();
    return cst;
}

void Parser::assignAddress(){
    std::cout<<"starting adressing of ast"<<std::endl;
    assignAddressHelper(cst->getRoot(), 0);
}
//function applies a address to every node in our ast
void Parser::assignAddressHelper(CSTNode *root, int address) {

    //if nullptr then return
    if ( root == nullptr ) {
        return;
    }
    //if right is not null, then traverse to the right sibling
    if (root->getRight() != nullptr ){

        root->setLocation(address);
        std::cout<<"address: "<<address<<" token: "<<root->getToken().getTokenString()<<std::endl;
        assignAddressHelper(root->getRight(), address+1);

        //if left is not nullptr traverse left sibling
    }else if ( root->getLeft() != nullptr ){

        //set address of the token in the node to the current address
        root->setLocation(address);
        std::cout<<"address: "<<address<<" token: "<<root->getToken().getTokenString()<<std::endl;

        //if the token is a function wee need to look it up
        if(root->getToken().isFunction()){

            //if the token is main thenwe need to look up
            if (root->getToken().isMain()){

                std::cout<<"address to main: "<< root->getToken().getFunctionName() <<std::endl;
                symbol_table_list.setAddress( symbol_table_list.lookupSymbol( root->getToken().getFunctionName() ), address);
            
                SymbolLocation = "main";
                //check to see if address is set properly.

                //push the main function onto the stack to get us started when interpreting.
                callStack.push_back(address);

            }else{
                // add the function to our function table
                addFunction(root->getToken().getFunctionName(),address);

                std::cout<<"address to function: "<< root->getToken().getFunctionName() <<std::endl;
                symbol_table_list.setAddress( symbol_table_list.lookupSymbol( root->getToken().getFunctionName() ), address);
            }

        }
        address++;

        //So we know how many addresses/elements we have
        highestAddress = address - 1;

        assignAddressHelper(root->getLeft(), address);
    }

}

//look up function that checks our list of stored functions and there memory addresses.
int Parser::lookUpFunction(std::string functionName){
    
    //search our addres vector
    for (int i = 0; i < functionAddresses.size(); i++) {
        //std::cout<< std::get<0>(functionAddresses[i]) <<std::endl;

        //if the function name matches the identifier
        if(std::get<0>(functionAddresses[i]) == functionName){
            
            std::cout<< "found function in address book: "<<std::get<0>(functionAddresses[i])<<std::endl;

            //return the memory location of that function.
            return std::get<1>(functionAddresses[i]);

        }
    }

    //fail case, if it could not be found return -1
    return -1;
}


std::string Parser::lookUpFunctionFromAddress(int addressloc){

    //search our addres vector
    for (int i = 0; i < functionAddresses.size(); i++) {
        //std::cout<< std::get<0>(functionAddresses[i]) <<std::endl;

        //if the function name matches the identifier
        if(std::get<1>(functionAddresses[i]) == addressloc){
            
            std::cout<< "found function in address book: "<<std::get<1>(functionAddresses[i])<<std::endl;

            //return the memory location of that function.
            return std::get<0>(functionAddresses[i]);

        }
    }

        //fail case, if it could not be found return -1
        std::cout<< "couldnot find function at address: "<<addressloc<<std::endl;
        throw;
        return "";
}

//adds a function name and memory location
void Parser::addFunction(std::string functionName, int addressLoc){

    std::cout<< "adding: "<<functionName<<" to table with memory address of: "<<addressLoc<<std::endl;
    //make a tuple with the new address and mem location
    functionAddresses.push_back(std::make_tuple(functionName, addressLoc));

}

void Parser::postFixEval(std::vector<Token> postfix){
    
    //stack to store operations
    std::stack<int> evalStack;

    //loop through vector of tokens
     for (int i = 0; i < postfix.size(); i++) {

        //If it's an operand, push its value to the stack
        if(postfix[i].isIdentifier() || postfix[i].isInt() || postfix[i].isDouble()) {
            if (postfix[i].isInt() || postfix[i].isDouble()) {
                int value = std::stoi(postfix[i].getTokenString());
                evalStack.push(value);
            }else{

            }
        }
            //If it's an operator, evaluate the expression
        else if (postfix[i].isMinus() || postfix[i].isPlus() ||
                postfix[i].isModulo() || postfix[i].isAsterisk() ||
                postfix[i].isDivide()) {

            // Perform the operation based on the operator
            int operand2 = evalStack.top();
            evalStack.pop();
            int operand1 = evalStack.top();
            evalStack.pop();

            if (postfix[i].isPlus()) {
                evalStack.push(operand1 + operand2);
            }
            else if (postfix[i].isMinus()) {
                evalStack.push(operand1 - operand2);
            }
            else if (postfix[i].isAsterisk()) {
                evalStack.push(operand1 * operand2);
            }
            else if (postfix[i].isDivide()) {
                if (operand2 == 0) throw std::runtime_error("Division by zero error!");
                evalStack.push(operand1 / operand2);
            }
        } else if (postfix[i].isAssignmentOperator()){
            //Need to finish
            //symbol_table_list.lookupSymbol( root->getToken().getFunctionName()
        }
     
     }

    

}

void Parser::interpret() {

    //gets the root of the cst
    CSTNode* root = cst->getRoot();

    //important variable to keep track of scope.
    int blockScope = 0;

    //define the node of main to start interpret. address should be on stack so set node to the search of that address
    CSTNode* mainNode = cst->getNodeAtAddress(callStack.back());

    //save the address where this call started.
    int callStartAddress = callStack.back();

    //important for figuring out when to stop interpreting. useful to keep track of function call interps as well.
    int callStackScope = callStack.size();

    //tester to see where we currently are interpreting.
    if(callStack.size()< 2){
        std::cout << "main Address is: " << callStack.back() << std::endl;
    }else{
        std::cout << "function Address is: " << callStack.back() << std::endl;
    }
    
    //we want to loop until we hit a return, or fall out of scope. that will happen when we pop off the current callstack.back()
    while (callStackScope == callStack.size()){

        //saftey net incase we hit a nullptr?
        if (mainNode != nullptr) {

            //the current node we are checking
            CSTNode *currentNode = cst->getNodeAtAddress(callStack.back());

            //for testing purposes to see what token we are looking at.
            std::string statement = currentNode->getToken().getTokenString();

            //if we find a begin block want to increase our blockScope by one
            if (statement == "BEGIN BLOCK") {

                blockScope++;
                std::cout << "BEGIN BLOCK ADDRESS: " << callStack.back() <<" statement: "<<statement<<" blockScope: " << blockScope << std::endl;
                callStack.back()++;

            //like wise if we find a end block then we de increment the blockScope
            }else if (statement == "END BLOCK") {

                blockScope--;
                std::cout << "END BLOCK ADDRESS: " << callStack.back() <<" statement: "<<statement<<" blockScope: " << blockScope << std::endl;
                callStack.back()++;

            //for a declaration, just keep moving?
            }else if (statement == "DECLARATION") {

                std::cout << "DECLARATION ADDRESS: " << callStack.back() << std::endl;
                callStack.back()++;

            //assignment of a variable.
            }else if (statement == "ASSIGNMENT") {
                
                //print out token with label assignment
                std::cout << "ASSIGNMENT ADDRESS: " << callStack.back() << std::endl;
                callStack.back()++;
                
                //grab the node at the next position to operate on.
                currentNode = cst->getNodeAtAddress(callStack.back());

                //vector to store the postfix tokens
                std::vector<Token> postFix;

                //loop on the cst node until it no longer has any left siblings. this loop get is all the componnents of the assignment operation.
                while(currentNode->getRight() != nullptr){
                    
                    //print out components of the of the assignment operation
                    statement = currentNode->getToken().getTokenString();
                    std::cout << "ASSIGNMENT PART ADDRESS: " << callStack.back() <<" statement: "<<statement<<std::endl;
                    
                    //if we find a function call in a assignment, then we need to perform some recursion
                    //call our look up function to find out if the given token is a function in our function list
                    int foundFunction = lookUpFunction(currentNode->getToken().getTokenString());

                    //if the value does not return -1 then we have found a function call.
                    if(foundFunction != -1){

                        //move past the token that resembles the function call
                        callStack.back()++;
                        //then move past the token that resembles the open paren
                        callStack.back()++;

                        //grab next node that is the first function call parameter.
                        currentNode = cst->getNodeAtAddress(callStack.back());

                        //then loop through the params, so long as we dont get a close paren
                        while(currentNode->getToken().getTokenString() != ")"){

                            //print out the function call parameters
                            std::cout<<"found param for function call: "<<currentNode->getToken().getTokenString()<<std::endl; 

                            //move to the next element in memory
                            callStack.back()++;
                            currentNode = cst->getNodeAtAddress(callStack.back());

                            //set the paramlist to the variable. if a variable is a param then lookup that variable
                            //and set it to the param at this position.
                        }

                        //so we want to push the function memory address to the top of the callstack. 
                        callStack.push_back(foundFunction);

                        //then we can recursively call interpret to interpret the function call.
                        interpret();
                        //after the function call we are able to resume at the same place in memory regardless of where the funtion is called.
                        //in a assignment.

                        //store the result of the function call in a token

                        //add that token to the vector


                        
                    }else{

                        //if not a function call push token to vector.
                        postFix.push_back(currentNode->getToken());
                    }


                    
                    //increment the programming counter.
                    callStack.back()++;

                    //move to next left sibling
                    currentNode = cst->getNodeAtAddress(callStack.back());
                }

                //print out components of the of the assignment operation.
                statement = currentNode->getToken().getTokenString();
                std::cout << "ASSIGNMENT PART ADDRESS: " << callStack.back() <<" statement: "<<statement<<std::endl;

                //UNFINISHED HERE! 
                //the components above should make up some list which an arithmatic operation can be performed on
                //likely you will need to use the symbol table to keep track of the values above, atlest in the case of the function call.
                //then put them in some container that can operate on the components to set the result in the correct variable in our symbol table

                //call postfixeval to do operations and set value to the variable in the symbol table
                postFixEval(postFix);

                //increment the call stack
                callStack.back()++;

                
            //handles the logi of a iff statement.
            }else if (statement == "IF") {
                
                statement = currentNode->getToken().getTokenString();
                std::cout << "IF ADDRESS: " << callStack.back() <<" statement: "<<statement<<std::endl;

                //move past the if identifier.
                callStack.back()++;
                
                //grab the node at the next position to operate on.
                currentNode = cst->getNodeAtAddress(callStack.back());

                //loop on the cst node until it no longer has any left siblings
                while(currentNode->getRight() != nullptr){

                    statement = currentNode->getToken().getTokenString();
                    std::cout << "IF PART ADDRESS: " << callStack.back() <<" statement: "<<statement<<std::endl;

                    //increment the programming counter.
                    callStack.back()++;

                    //move to next left sibling
                    currentNode = cst->getNodeAtAddress(callStack.back());

                }
                //once we find the node with a null right sibling, we also want to ad that as well to our if parts
                statement = currentNode->getToken().getTokenString();
                std::cout << "IF PART ADDRESS: " << callStack.back() <<" statement: "<<statement<<std::endl;

                //UNFINISHED HERE!
                //similiar to the opperation for assignment but for boolean opperation. need to evaluate the bool expression then set
                //the value if true or false to the variable below. that way we can either skip the if statements scope, or
                //know to evaluate the ontents of that if expression.

                //operate on the stack to get a result. VALUE IS set to basic value for testing.
                bool boolExpression = false; 

                // now that we have all part of the boolean expression shift forward to the parenthesis
                callStack.back()++;
                currentNode = cst->getNodeAtAddress(callStack.back());
                statement = currentNode->getToken().getTokenString();

                //if result is true then  operate on this blocking scope like normal
                if(boolExpression == true){

                    std::cout << "bool expression true"<< std::endl;

                //else if its false, then skip that block, using the block scoping to skip it
                }else{

                    std::cout << "bool expression false"<< std::endl;

                    //important! the token we would be on if we didnt increment by one is a begin block
                    //this is bad because it messes up the skip function if bool is false.
                    //before we loop increment by one to simulate eating the begin block
                    callStack.back()++;

                    //set the temp scope variable to tell when we should stop ignoring code in the scope of the if statement.
                    int targetScope = blockScope;
                    
                    //increase the scope by 1 to resemble eating the open paren.
                    blockScope++;

                    //std::cout << "blockScope: "<<blockScope<<" targetScope: "<<targetScope<< std::endl;

                    //while the blockScope is greater than the targetScope
                    while(blockScope > targetScope){

                        // upodate node contents
                        currentNode = cst->getNodeAtAddress(callStack.back());
                        statement = currentNode->getToken().getTokenString();

                        //mock begin case. functions like the begin block case but specially tailored for skipping things in this scope
                        if (statement == "BEGIN BLOCK") {

                            blockScope++;
                            std::cout << "BEGIN BLOCK in if skip ADDRESS: " << callStack.back() <<"blockScope: " << blockScope << std::endl;

                        //mock end case. functions like the begin block case but specially tailored for skipping things in this scope
                        }else if (statement == "END BLOCK") {
                            
                            blockScope--;
                            std::cout << "END BLOCK in if skip ADDRESS: " << callStack.back() <<" blockScope: " << blockScope << std::endl;
                        }
                        
                        //move past the end block
                        callStack.back()++;
                    }
                }

                //move the token forward.
                currentNode = cst->getNodeAtAddress(callStack.back());
                statement = currentNode->getToken().getTokenString();
                std::cout <<"statement: "<<statement<<std::endl;
                std::cout<<"current token: "<<currentNode->getToken().getTokenString()<<" blockScope: "<<blockScope<<std::endl;

            //handle else case
            }else if (statement == "ELSE") {
                // UNFINISHED HERE!
                //suggestion, the above if case handles when to skip the scope of a if statement. we could add some way of tracking if
                //we are checking if cases with this case reseting that variable? may need to reset that value if other cases are ether incase if is by its self.
                //also in this case, we need to check if there is a if after this else. if thats the case, then just skip this case and have the 
                //if land into our if case above

            //handle return case.
            }else if (statement == "RETURN") {
                // UNFINISHED HERE!
                //we need to update the symbol table values when we return. and ensure they go to the correct places

                //get the next token which should ve the symbol or value we are returning
                callStack.back()++;
                currentNode = cst->getNodeAtAddress(callStack.back());
                statement = currentNode->getToken().getTokenString();

                //if its a variable then return that value
                if(currentNode->getToken().isIdentifier() == true){

                    std::cout<<"found identifier to return."<<std::endl;

                    //search the symbol table for our return value
                    SymbolNode* symbolToReturn = symbol_table_list.lookupSymbolAtLocation(currentNode->getToken().getTokenString(),lookUpFunctionFromAddress(callStartAddress));
                    //set the return value in our parser to the symbols value we found.
                    returnValue = symbolToReturn->variableVal;

                    std::cout<<"returnValue: "<<returnValue<<std::endl;

                }

                //then pop the current mem of the call stack stopping the loop of this function since we are finished in this call
                callStack.pop_back();
                std::cout<<"REACHED RETURN CALL."<<std::endl;
            
            //handles printing of output once we are finished.
            }else if (statement == "PRINTF") {

                // UNFINISHED HERE!
                //need to determine if we are printing a string by its self or variables? 
                //may need to parse through token string to put variable in the correct places. 

                //std::cout<<"attempting print."<<std::endl;

            }else if (statement == "FOR EXPRESSION 1") {
                // UNFINISHED HERE!
                //should just store a value into our symbol table 

            }else if (statement == "FOR EXPRESSION 2") {
                // UNFINISHED HERE!
                //challenging part. my thoughts, we could implement something similiar to the way the callstack works
                //where we have a vector to keep track of the initial address of the foor loop here at this boolean case.
                //then keep track of the scope of the for loop similiar to the way i handled the if case, where if expression is true
                //evaluate until we run out of the for loops scope, then jump back to the for loops address.
                //and on false we simply skip the for loops scope.

            }else if (statement == "FOR EXPRESSION 3") {
                // UNFINISHED HERE!
                //increment the correct variable in the symbol table.

            }
            //std::cout << std::endl;
        }
    }
}




/*void Parser::performArithmetic(CSTNode* root) {
    if (root == nullptr) {
        //return;
        exit(1);
    }

    std::stack<int> arithmeticStack;

    evalNode(root, arithmeticStack);

    stack.push(arithmeticStack.top());
}

void Parser::evalNode(CSTNode* node, std::stack<int>& evalStack){
    if (node == nullptr) return;

    //forgot the
    //If it's an operand, push its value to the stack
    if (node->getToken().isIdentifier() || node->getToken().isInt() || node->getToken().isDouble()) {
        if (node->getToken().isInt() || node->getToken().isDouble()) {
            int value = std::stoi(node->getToken().getTokenString());
            evalStack.push(value);
        }else{

        }
    }
        //If it's an operator, evaluate the expression
    else if (node->getToken().isMinus() || node->getToken().isPlus() ||
             node->getToken().isModulo() || node->getToken().isAsterisk() ||
             node->getToken().isDivide()) {

        // Perform the operation based on the operator
        int operand2 = evalStack.top();
        evalStack.pop();
        int operand1 = evalStack.top();
        evalStack.pop();

        if (node->getToken().isPlus()) {
            evalStack.push(operand1 + operand2);
        }
        else if (node->getToken().isMinus()) {
            evalStack.push(operand1 - operand2);
        }
        else if (node->getToken().isAsterisk()) {
            evalStack.push(operand1 * operand2);
        }
        else if (node->getToken().isDivide()) {
            if (operand2 == 0) throw std::runtime_error("Division by zero error!");
            evalStack.push(operand1 / operand2);
        }
    } else if (node->getToken().isAssignmentOperator()){
        //Need to finish
    }
}*/

#include "std_lib_facilities.h"

// from exercise07_05.cpp

/*
This program implements a basic expression calculator.
Input from cin; output to cout.
The grammar for input is:

Calculation:
    Statement
    Print
    Quit
    Calculation Statement

 Statement:
    Declaration
    Expressionifs
    
Declaration:
    "let" Name = Expression
    "const" Name = Expression

----------------------------------------------

Print:
    ;
    \n
Quit:
    Q

predefined var_table
k : 1000
pi : 3.14159265358979
e : 2.718281828459

----------------------------------------------
 
Expression:
    Term
    Expression + Term
    Expression – Term
Term:
    Primary
    Term * Primary
    Term / Primary

Primary:
      Number
      ( Expression )
       – Primary
       + Primary
       Name
       Function ( Expression )

Function:
    "sqrt"
    "pow"     

Number:
    floating-point-literal


 Input comes from ifs through the Token_stream called ts.
*/

#include "std_lib_facilities.h"

const char let_char = 'L';
const char const_char = 'C';
const char quit = 'Q';            // t.kind == quit means that t is a quit Token
const char print = ';';           // t.kind == print means that t is a print Token
const char number = '8';          // t.kind == number means that t is a number Token
const char name = 'a';            // name token
const char sqrtfunc = 's';        // sqrt function token
const char powfunc = 'p';         // power function token
const string letkey = "let";      // declaration keyword for let
const string constkey = "const";  // declaration keyword for const
const string quitkey = "exit";    // quit keyword set to "exit"  drill 7-11
const string sqrtkey = "sqrt";    // sqrt keyword
const string powkey = "pow";      // pow keyword

// ifstream ifs{""};
string iname{"exercise10_10_input.txt"};
string oname{"exercise10_10_output.txt"};
ifstream ifs{iname};
ofstream ofs{oname};  // str.c_str(): string to const char*

struct Token {
    char kind;
    double value;  // for numbers: a value
    string name;
    Token() : kind{0} {}                                  // default constructor
    Token(char ch) : kind{ch} {}                          // initialize kind with ch
    Token(char ch, double val) : kind{ch}, value{val} {}  // initialize kind and value
    Token(char ch, string n) : kind{ch}, name{n} {}       // initialize kind and name
};

class Token_stream {
   public:
    Token_stream() : full{false}, buffer{0} {}

    Token get();         // get a Token
    void unget(Token t)  // put a Token back
    {
        buffer = t;   // copy t to buffer
        full = true;  // buffer is now full
    }
    void ignore(char c);  // discard characters up to and including a c
   private:
    bool full;
    Token buffer;
};

Token Token_stream::get() {
    if (full)  // check if we already have a Token ready
    {
        full = false;
        return buffer;
    }
    char ch;
    ifs >> ch;  // note that >> skips whitespace (space, newline, tab, etc.)

    switch (ch) {
        case ';':
        case '\n':
            return Token(print);

        case quit:
        case '(':
        case ')':
        case '+':
        case '-':
        case '*':
        case '/':
        case '=':              // used for declaration()
        case ',':              // function argument vector separator
            return Token(ch);  // let each character represent itself

        case '.':  // a floating-point literal can start with a dot
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':  // numeric literals 0 to 9
        {
            ifs.unget();  // put digit back into the input stream
            double val;
            ifs >> val;
            return Token(number, val);
        }

        default:
            if (isalpha(ch) || ch == '_')  // to allow underscores in the variable var_table
            {
                string s;
                s += ch;
                // to allow underscores in the variable var_table
                while (ifs.get(ch) && (isalpha(ch) || isdigit(ch) || ch == '_')) {
                    s += ch;
                }
                ifs.unget();
                if (s == letkey)
                    return Token{let_char};  // declaration keyword
                else if (s == constkey)
                    return Token{const_char};  // declaration keyword
                else if (s == quitkey)
                    return Token{name};
                else if (s == sqrtkey)
                    return Token{sqrtfunc};
                else if (s == powkey)
                    return Token{powfunc};

                return Token{name, s};
            }

            error("Bad token");
    }
    // to deal with : control reaches end of non-void function [-Wreturn-type]
    return Token{};
}

// c represents the kind of Token
void Token_stream::ignore(char c) {
    // first look in buffer:
    if (full && c == buffer.kind) {
        full = false;
        return;
    }
    full = false;

    // now search input:
    char ch{' '};
    while (ifs >> ch) {
        if (ch == c)
            return;
    }
}

struct Variable {
    string name;
    double value;
    bool is_const;
    Variable(string n, double v, bool b) : name(n), value(v), is_const(b) {}
};

class Symbol_table {
   public:
    vector<Variable> var_table;
    double get_value(string s);                          // return the value of the Variable named s
    void set_value(string s, double d);                  // set the Variable named s to d
    bool is_declared(string s);                          // is var already in var_table?
    double define_name(string var, double val, bool b);  // add {var, val} to var_table
};

double Symbol_table::get_value(string s) {
    for (int i = 0; i < var_table.size(); ++i)
        if (var_table[i].name == s)
            return var_table[i].value;

    error("get: undefined name ", s);
    return 3;
}

void Symbol_table::set_value(string s, double d) {
    for (int i = 0; i <= var_table.size(); ++i)
        if (var_table[i].name == s) {
            var_table[i].value = d;
            return;
        }
    error("set: undefined name ", s);
}

bool Symbol_table::is_declared(string s) {
    for (int i = 0; i < var_table.size(); ++i)
        if (var_table[i].name == s)
            return true;
    return false;
}

double Symbol_table::define_name(string var, double val, bool b) {
    if (is_declared(var))
        error(var, " declared twice");
    var_table.push_back(Variable{var, val, b});
    return val;
}

//------------------------------------------------------------------------------

Token_stream ts;      // provides get() and putback()
Symbol_table st;      // provides get_value(), set_value(), is_declared(), and define_name()
double expression();  // declaration so that primary() can call expression()

//------------------------------------------------------------------------------

// evaluates function of t.kind == c
double eval_function(char c) {
    // for function arguments
    vector<double> func_args;

    Token t = ts.get();
    if (t.kind != '(')
        error("'(' expected after function");

    switch (c) {
        case sqrtfunc:
            func_args.push_back(expression());
            t = ts.get();
            if (t.kind != ')')
                error("')' expected after function");
            if (func_args[0] < 0)
                error("square root of a negative number is inappropriate");
            return sqrt(func_args[0]);

        case powfunc:
            func_args.push_back(expression());
            t = ts.get();
            if (t.kind != ',')
                error("Please provide proper pow function arguments e.g., pow(2, 3)");
            func_args.push_back(expression());
            t = ts.get();
            if (t.kind != ')')
                error("')' expected after function");
            // if(args[1]-narrow_cast<int>(args[1])!=0)
            // error();
            return pow(func_args[0], func_args[1]);

        default:
            error("Function you called is not implemeted yet");
            return 5;
    }
}

// deal with numbers and parentheses
double primary() {
    Token t = ts.get();
    switch (t.kind) {
        case '(': {  // handle '(' expression ')'
            double d = expression();
            t = ts.get();
            if (t.kind != ')')
                error("')' expected");
            return d;
        }

        case '+':  // handle unary plus
        {
            return primary();
        }

        case '-':  // handle unary minus
            return -primary();

        case number:
            return t.value;  // return the number's value

        case name:
            return st.get_value(t.name);

        // if function is called, let eval_function handles it
        case sqrtfunc:
        case powfunc:
            return eval_function(t.kind);

        default:
            error("primary expected");
            return 4;
    }
}

// deal with * and /
double term() {
    double left = primary();
    Token t = ts.get();  // get the next Token from the Token stream
    while (true) {
        switch (t.kind) {
            case '*':
                left *= primary();
                t = ts.get();
                break;
            case '/': {
                double d = primary();
                if (d == 0)
                    error("/ : divide by zero");
                left /= d;
                t = ts.get();
                break;
            }
            default:
                ts.unget(t);  // put t back into the Token stream
                return left;
        }
    }
}

// deal with + and -
double expression() {
    double left = term();  // read and evaluate a Term
    Token t = ts.get();    // get the next Token from the Token stream

    while (true) {
        switch (t.kind) {
            case '+':
                left += term();  // evaluate Term and add
                t = ts.get();
                break;
            case '-':
                left -= term();  // evaluate Term and subtract
                t = ts.get();
                break;
            default:
                ts.unget(t);  // put t back into the Token stream
                return left;  // finally: no more + or -: return the answer
        }
    }
}

double declaration(char c) {
    Token t = ts.get();
    if (t.kind != name)
        error("name expected in declaration");
    string variable_name = t.name;
    if (st.is_declared(variable_name)) {
        switch (c) {
            case const_char:
                error(variable_name, " is declared already!\nYou cannot change value of the const");

            case let_char:
                for (int i = 0; i < st.var_table.size(); ++i) {
                    if (st.var_table[i].name == variable_name) {
                        // when the variable is declared as const already
                        if (st.var_table[i].is_const == true)
                            error(variable_name, " is declared as const already!\nYou cannot change value of the const");
                        Token t2 = ts.get();
                        if (t2.kind != '=')
                            error("= missing in declaration of ", variable_name);
                        double d = expression();
                        st.var_table[i].value = d;
                        return d;
                    }
                }
        }
    }

    Token t2 = ts.get();
    if (t2.kind != '=')
        error("= missing in declaration of ", variable_name);
    double d = expression();
    if (c == const_char)
        st.var_table.push_back(Variable(variable_name, d, true));
    else
        st.var_table.push_back(Variable(variable_name, d, false));
    return d;
}

double statement() {
    Token t = ts.get();
    switch (t.kind) {
        case let_char:
        case const_char:
            return declaration(t.kind);

        default:
            ts.unget(t);
            return expression();
    }
}

void clean_up_mess() {
    ts.ignore(print);
}

// const string prompt = "> ";
const string answer_str{"answer: "};
// const string result = "= ";

void calculate(ifstream &ifs, ofstream &ofs) {
    cout << "\n~~@ Welcome to our simple calculator @~~\n\n"
         << "Please enter expressions using floating-point numbers.\n"
         << "Operators at your disposal (+, -, *, and / )\n"
         << "and you can use parenthesis as well.\n"
         << "to set a variable by using : # variable name = n\n"
         << "(pow(n, m) and sqrt(n), predifined number pi and e \n"
         << "enter ';' for print the result and 'Q' for quit)\n\n";
    while (ifs)
        try {
            // ofs << prompt;
            ofs << answer_str;
            Token t = ts.get();
            while (t.kind == print) {
                t = ts.get();  // first discard all "prints"
            }
            if (t.kind == quit) return;
            ts.unget(t);
            // ofs << result << statement() << '\n';
            ofs << statement() << '\n';
        } catch (runtime_error &e) {
            ofs << e.what() << '\n';
            clean_up_mess();
        }
}

int main() {
    try {
        // predifine var_table:
        st.define_name("k", 1000, true);
        st.define_name("pi", 3.14159265358979, true);
        st.define_name("e", 2.718281828459, true);

        calculate(ifs, ofs);

        ifs.close();
        ofs.close();
        return 0;
    } catch (exception &e) {
        cerr << "exception: " << e.what() << endl;
        char c;
        while (ifs >> c && c != ';')
            ;
        return 1;
    } catch (...) {
        cerr << "exception\n";
        char c;
        while (ifs >> c && c != ';')
            ;
        return 2;
    }
}
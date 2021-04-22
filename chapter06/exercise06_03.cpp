// a calculator program of chapter 6
// add a factorial operator: use a suffix '!'
// Make ! bind tighter than * and /

// for doing this exersice, we need to modify the grammar
// add secondary between the primary and the term
// then change all primary to secondary in Term:

#include "std_lib_facilities.h"

//------------------------------------------------------------------------------

class Token
{
public:
    char kind;
    double value;  // for numbers: a value
    Token(char ch) // make a Token from a char
        : kind(ch), value(0)
    {
    }
    Token(char ch, double val) // make a Token from a char and a double
        : kind(ch), value(val)
    {
    }
};

//------------------------------------------------------------------------------

class Token_stream
{
public:
    Token get();           // get a Token
    void putback(Token t); // put a Token back
private:
    bool full{false}; // is there a Token in the buffer?
    Token buffer{0};  // putback() saves its token here
};

void Token_stream::putback(Token t)
{
    if (full)
        error("putback() into a full buffer");
    buffer = t;  // copy t to buffer
    full = true; // buffer is now full
}

Token Token_stream::get()
{
    if (full)
    {                 // do we already have a Token ready?
        full = false; // remove Token from buffer
        return buffer;
    }
    char ch;
    cin >> ch; // note that >> skips whitespace (space, newline, tab, etc.)

    switch (ch)
    {
    case '=': // for "print"
    case 'x': // for "exit"
    case '(':
    case ')':
    case '{':
    case '}':
    case '+':
    case '-':
    case '*':
    case '/':
    case '!':
        return Token(ch); // let each character represent itself
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
    {
        cin.putback(ch); // put digit back into the input stream
        double val;
        cin >> val;             // read a floating-point number
        return Token('8', val); // let '8' represent "a number"
    }
    default:
        error("Bad token");
    }
}

//------------------------------------------------------------------------------

Token_stream ts;     // provides get() and putback()
double expression(); // declaration so that primary() can call expression()

//------------------------------------------------------------------------------

double primary() // deal with numbers and parentheses
{
    Token t = ts.get();

    switch (t.kind)
    {
    case '{': // handle '{' expression '}'
    {
        double d = expression();
        t = ts.get();
        if (t.kind != '}')
            error("'}' expected");
        return d;
    }
    case '(': // handle '(' expression ')'
    {
        double d = expression();
        t = ts.get();
        if (t.kind != ')')
            error("')' expected");
        return d;
    }

    case '8':           // we use '8' to represent a number
        return t.value; // return the number's value

    default:
        error("primary expected");
    }
}

//------------------------------------------------------------------------------

double secondary() // handle a factorial operator
{
    double left = primary();
    Token t = ts.get();

    if (t.kind == '!')
    {
        int x = int(left);
        if (x == 0)
        {
            x = 1;
        }
        else if (x < 0)
        {
            error("you cannot provide negative integer for factorial operation\n");
        }
        /*
        else {
            int count = x - 1;
            while (count != 0) {
                x *= count--;
            }
        }
        */
        else
        {
            for (int i = x - 1; i > 0; --i)
            {
                x *= i;
            }
        }
        return double(x);
    }
    ts.putback(t); // put t back into the Token stream
    return left;   // if it is just a number pass it through to the next step
}

//------------------------------------------------------------------------------

double term() // deal with * and /
{
    double left = secondary();
    Token t = ts.get(); // get the next Token from the Token stream

    while (true)
    {
        switch (t.kind)
        {
        case '*':
            left *= secondary();
            t = ts.get();
            break;
        case '/':
        {
            double d = secondary();
            if (d == 0)
                error("divide by zero");
            left /= d;
            t = ts.get();
            break;
        }
        default:
            ts.putback(t); // put t back into the Token stream
            return left;
        }
    }
}

//------------------------------------------------------------------------------

double expression() // deal with + and -
{
    double left = term(); // read and evaluate a Term
    Token t = ts.get();   // get the next Token from the Token stream
    while (true)
    {
        switch (t.kind)
        {
        case '+':
            left += term(); // evaluate Term and add
            t = ts.get();
            break;
        case '-':
            left -= term(); // evaluate Term and subtract
            t = ts.get();
            break;
        default:
            ts.putback(t); // put t back into the Token stream
            return left;   // finally: no more + or -: return the answer
        }
    }
}

//-----------------------------------------------------------------------

int main()
{ // main loop and deal with errors
    try
    {
        cout << "Welcome to our simple calculator.\n"
             << "Please enter expressions using floating-point numbers.\n"
             << "Operators at your disposal (+, -, *, and / )\n"
             << "and you can use parenthesis as well.\n"
             << "(enter '=' for print the result and 'x' for exit)\n\n";
        double val = 0;
        while (cin)
        {
            Token t = ts.get();
            if (t.kind == 'x') // 'x' for "exit"
                break;
            if (t.kind == '=') // '=' for "print now"
                cout << "=" << val << '\n';
            else
                ts.putback(t);
            val = expression();
        }
        keep_window_open("~0");
    }
    catch (exception &e)
    {
        cerr << e.what() << endl;
        keep_window_open("~1");
        return 1;
    }
    catch (...)
    {
        cerr << "exception \n";
        keep_window_open("~2");
        return 2;
    }
}

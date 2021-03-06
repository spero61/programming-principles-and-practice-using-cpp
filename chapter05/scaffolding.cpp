// "scaffolding" for drill on Chapter 05 : Errors
#include "std_lib_facilities.h"

int main()
{
    try
    {
        /* your code here */
        keep_window_open();
        return 0;
    }
    catch (exception &e)
    {
        cerr << "error: " << e.what() << '\n';
        keep_window_open();
        return 1;
    }
    catch (...)
    {
        cerr << "Oops: unknown exception!\n";
        keep_window_open();
        return 2;
    }
}
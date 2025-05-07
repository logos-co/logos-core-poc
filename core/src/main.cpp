#include "logos_core.h"

int main(int argc, char *argv[])
{
    // Initialize the logos core library
    logos_core_init(argc, argv);
    
    // Start the logos core functionality
    logos_core_start();
    
    // Run the event loop and return the result
    return logos_core_exec();
} 
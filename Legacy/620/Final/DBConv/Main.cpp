#include "DBConverter.h"
#include <stdio.h>

void main() {

    DBConverter dbConv;
    ConfigFile cfConfig;

    if (cfConfig.Open ("DBConv.conf") == OK && 
        dbConv.Open (&cfConfig) == OK &&
        dbConv.Convert() == OK) {

        printf ("Database conversion terminated successfully\n");
        printf ("%i tables were converted\n", dbConv.GetNumTablesConverted());

    } else {

        printf ("Error processing database conversion\n");
    }

    //::MessageBox (NULL, "OK", NULL, MB_OK);
}
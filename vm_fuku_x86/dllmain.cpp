
#include "stdafx.h"

/*
int main() {
    
    test_arith();
 
    return 0;
}
//*/


BOOL APIENTRY main_dll( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call) {

    case DLL_PROCESS_ATTACH: {

        break;
    }

    case DLL_THREAD_ATTACH: {

        break;
    }

    case DLL_THREAD_DETACH: {

        break;
    }

    case DLL_PROCESS_DETACH: {

        break;
    }


    default:
        break;
    }
    return TRUE;
}

//*/
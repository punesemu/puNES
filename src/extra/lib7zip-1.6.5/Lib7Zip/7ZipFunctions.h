#ifndef __7ZIP_FUNCTIONS_H__
#define __7ZIP_FUNCTIONS_H__

typedef union _union_7zip_functions
{
    unsigned char data[sizeof(void *) * 7];

    struct
    {
        GetMethodPropertyFunc GetMethodProperty;
        GetNumberOfMethodsFunc GetNumberOfMethods;
        GetNumberOfFormatsFunc GetNumberOfFormats;
        GetHandlerPropertyFunc GetHandlerProperty;
        GetHandlerPropertyFunc2 GetHandlerProperty2;
        CreateObjectFunc CreateObject;
        SetLargePageModeFunc SetLargePageMode;

        bool IsValid()
        {
            return GetMethodProperty != NULL &&
                GetNumberOfMethods != NULL &&
                CreateObject != NULL; 
        }
    } v;
} U7ZipFunctions, * pU7ZipFunctions;

#endif // __7ZIP_FUNCTIONS_H__

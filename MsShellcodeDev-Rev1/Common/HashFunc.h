typedef union
{
    struct
    {
        BYTE bitCase : 1;
        BYTE bitWide : 1;
        BYTE bitRsvd : 6;
    } bitVal;
    BYTE byteVal;
} HASH_OPT, *PHASH_OPT;

extern "C" DWORD WINAPI InnerHash(DWORD, DWORD, BOOL);
extern "C" DWORD WINAPI GetStrHash(LPSTR, PHASH_OPT);

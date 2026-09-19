DWORD WINAPI InnerHash(DWORD dwHash, DWORD dwChar, BOOL bSense)
{
	if (!bSense)
	{
		if (dwChar>0x40 && dwChar<0x5B)
			dwChar |= 0x20;
	}
	DWORD dw2ROL = dwChar & 0x1f, dw2Chk = (dwChar >> 5) & 3;
	DWORD ret = (dwHash << dw2ROL) | (dwHash >> (32 - dw2ROL));
	if (dw2Chk == 3)
		dwChar = ~dwChar;
	return (ret ^ dwChar);
}

DWORD WINAPI GetStrHash(LPSTR pStr, PHASH_OPT pOpt)
{
    DWORD
        dwGap = 1,
        dwRet = 0;
    BOOL
        bSense = (pOpt->bitVal.bitCase == 1),
        bIsWide = (pOpt->bitVal.bitWide == 1);
    if (bIsWide)
        dwGap += 1;
    while (*pStr)
    {
        dwRet = InnerHash(dwRet, (DWORD)*pStr, bSense);
        pStr += dwGap;
    }
    return dwRet;
}
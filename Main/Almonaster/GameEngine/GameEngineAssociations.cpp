//
// Almonaster.dll:  a component of Almonaster
// Copyright (c) 1998 Max Attar Feingold (maf6@cornell.edu)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include "GameEngine.h"

// An association is a string containing empire keys separated by semicolons
// We could have created a new table to keep then, but the synchronization
// was tricky

int GameEngine::GetAssociations (unsigned int iEmpireKey, unsigned int** ppiEmpires, unsigned int* piNumAssoc) {

    int iErrCode;
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);

    Variant vAssoc;
    iErrCode = t_pCache->ReadData(strEmpires, iEmpireKey, SystemEmpireData::Associations, &vAssoc);
    if (iErrCode != OK) {
        Assert (false);
        return iErrCode;
    }

    return GetAssociations((char*) vAssoc.GetCharPtr(), ppiEmpires, piNumAssoc);
}

int GameEngine::GetAssociations (char* pszAssoc, unsigned int** ppiEmpires, unsigned int* piNumAssoc) {

    *ppiEmpires = NULL;
    *piNumAssoc = 0;

    if (!String::IsBlank (pszAssoc)) {

        // Count associations
        unsigned int iAssoc = 1;
        char* pszCursor = pszAssoc;

        while (true) {
            pszCursor = strstr (pszCursor + 1, ";");
            if (pszCursor == NULL) break;

            iAssoc ++;
        }

        unsigned int* piAssoc = (unsigned int*) OS::HeapAlloc (iAssoc * sizeof (unsigned int));
        if (piAssoc == NULL) {
            return ERROR_OUT_OF_MEMORY;
        }
        iAssoc = 0;

        // Make sure the association doesn't exist already
        const char* pszToken = strtok (pszAssoc, ";");
        while (pszToken != NULL) {

            piAssoc [iAssoc ++] = String::AtoUI (pszToken);
            pszToken = strtok (NULL, ";");
        }

        *ppiEmpires = piAssoc;
        *piNumAssoc = iAssoc;
    }

    return OK;
}

int GameEngine::CheckAssociation (unsigned int iEmpireKey, unsigned int iSwitch, bool* pbAuth) {

    int iErrCode;
    GET_SYSTEM_EMPIRE_DATA(strEmpires, iEmpireKey);

    *pbAuth = false;

    Variant vAssoc;
    iErrCode = t_pCache->ReadData(strEmpires, iEmpireKey, SystemEmpireData::Associations, &vAssoc);
    if (iErrCode != OK) {
        return iErrCode;
    }
    char* pszAssoc = (char*) vAssoc.GetCharPtr();

    const char* pszToken = strtok (pszAssoc, ";");
    while (pszToken != NULL) {

        if (String::AtoUI (pszToken) == iSwitch) {
            *pbAuth = true;
            return OK;
        }
        pszToken = strtok (NULL, ";");
    }

    return OK;
}

int GameEngine::CreateAssociation (unsigned int iEmpireKey, const char* pszSecondEmpire, const char* pszPassword) {

    // TODOTODO - This will need to be rewritten
    int iErrCode;
    Variant vTemp;

    ICachedTable* pEmpires = NULL;

    iErrCode = t_pCache->GetTable(SYSTEM_EMPIRE_DATA, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    // Check for the first empire
    bool bExists;
    iErrCode = pEmpires->DoesRowExist(iEmpireKey, &bExists);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    if (!bExists) {
        iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
        goto Cleanup;
    }

    // Find the second empire
    unsigned int iSecondKey;
    iErrCode = pEmpires->GetFirstKey(SystemEmpireData::Name, pszSecondEmpire, &iSecondKey);
    if (iErrCode != OK) {
        if (iErrCode == ERROR_DATA_NOT_FOUND) {
            iErrCode = ERROR_EMPIRE_DOES_NOT_EXIST;
        }
        else Assert (false);
        goto Cleanup;
    }

    // Make sure they're not the same empire
    if (iSecondKey == iEmpireKey) {
        iErrCode = ERROR_DUPLICATE_EMPIRE;
        goto Cleanup;
    }

    // Check the second empire's password
    iErrCode = pEmpires->ReadData (iSecondKey, SystemEmpireData::Password, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }
    if (strcmp (pszPassword, vTemp.GetCharPtr()) != 0) {
        iErrCode = ERROR_PASSWORD;
        goto Cleanup;
    }

    // Read the two empire's associations, making local copies
    char* pszFirstAssoc, * pszSecondAssoc, * pszToken;

    iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Associations, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pszFirstAssoc = (char*)StackAlloc(String::StrLen(vTemp.GetCharPtr()) + 33);
    if (String::IsBlank(vTemp.GetCharPtr())) {
        pszFirstAssoc[0] = '\0';
    } else {

        strcpy (pszFirstAssoc, vTemp.GetCharPtr());

        // Make sure the association doesn't exist already
        pszToken = strtok (pszFirstAssoc, ";");
        Assert (pszToken != NULL);

        while (true) {

            unsigned int iAssoc = String::AtoUI (pszToken);
            if (iAssoc == iSecondKey) {
                iErrCode = ERROR_ASSOCIATION_ALREADY_EXISTS;
                goto Cleanup;
            }

            pszToken = strtok (NULL, ";");
            if (pszToken == NULL) break;

            // Restore the last semicolon
            pszToken[-1] = ';';
        }

        // Add a new separator
        strcat (pszFirstAssoc, ";");
    }

    iErrCode = pEmpires->ReadData(iSecondKey, SystemEmpireData::Associations, &vTemp);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    pszSecondAssoc = (char*)StackAlloc(String::StrLen (vTemp.GetCharPtr()) + 33);
    if (String::IsBlank(vTemp.GetCharPtr())) {
        pszSecondAssoc[0] = '\0';
    } else {

        strcpy(pszSecondAssoc, vTemp.GetCharPtr());

#ifdef _DEBUG
        // Do it for the other empire too
        pszToken = strtok (pszSecondAssoc, ";");
        Assert (pszToken != NULL);

        while (true) {

            unsigned int iAssoc = String::AtoUI (pszToken);
            if (iAssoc == iSecondKey) {
                iErrCode = ERROR_ASSOCIATION_ALREADY_EXISTS;
                goto Cleanup;
            }

            pszToken = strtok (NULL, ";");
            if (pszToken == NULL) break;

            // Restore the last semicolon
            *(pszToken - 1) = ';';
        }
#endif

        // Add a new separator
        strcat (pszSecondAssoc, ";");
    }

    // Create the new association
    char pszInt [32];

    strcat (pszFirstAssoc, String::UItoA (iSecondKey, pszInt, 10));
    strcat (pszSecondAssoc, String::UItoA (iEmpireKey, pszInt, 10));

    iErrCode = pEmpires->WriteData (iEmpireKey, SystemEmpireData::Associations, pszFirstAssoc);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = pEmpires->WriteData (iSecondKey, SystemEmpireData::Associations, pszSecondAssoc);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pEmpires);

    return iErrCode;
}

//
// Typical format:  1111;2222;3333
//
int GameEngine::DeleteAssociation (ICachedTable* pEmpires, unsigned int iEmpireKey, unsigned int iSecondEmpireKey) {

    int iErrCode;
    Variant vTemp;

    iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Associations, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (String::IsBlank (vTemp.GetCharPtr())) {
        return ERROR_ASSOCIATION_NOT_FOUND;
    }

    size_t stAssocLen = strlen (vTemp.GetCharPtr());
    char* pszAssociation = (char*) StackAlloc (stAssocLen + 1);
    strcpy (pszAssociation, vTemp.GetCharPtr());

    bool bFound = false;
    char* pszToken = strtok (pszAssociation, ";");
    Assert (pszToken != NULL);

    while (true) {

        if (String::AtoUI (pszToken) == iSecondEmpireKey) {

            size_t stTokenLen = strlen (pszToken);
            if (pszToken == pszAssociation) {

                // We're the first...
                if (stTokenLen == stAssocLen) {

                    // and also the last
                    pszAssociation = NULL;

                } else {

                    // but not the last
                    memmove (pszToken, pszToken + stTokenLen + 1, stAssocLen - stTokenLen);
                }

            } else {

                // We're not the first
                size_t stPast = pszToken - pszAssociation;
                if (stAssocLen - stPast == stTokenLen) {

                    // but we're the last
                    *(pszToken - 1) = '\0';

                } else {

                    // and not the last
                    memmove (pszToken, pszToken + stTokenLen + 1, stAssocLen - stPast - stTokenLen + 1);
                }
            }

            bFound = true;
            break;
        }

        pszToken = strtok (NULL, ";");
        if (pszToken == NULL) break;

        // Restore the last semicolon
        *(pszToken - 1) = ';';
    }

    if (!bFound) {
        return ERROR_ASSOCIATION_NOT_FOUND;
    }

    return pEmpires->WriteData (iEmpireKey, SystemEmpireData::Associations, pszAssociation);
}

int GameEngine::DeleteAssociation (unsigned int iEmpireKey, unsigned int iSecondEmpireKey) {

    int iErrCode;
    ICachedTable* pEmpires = NULL;

    iErrCode = t_pCache->GetTable(SYSTEM_EMPIRE_DATA, &pEmpires);
    if (iErrCode != OK) {
        Assert (false);
        goto Cleanup;
    }

    iErrCode = DeleteAssociation (pEmpires, iEmpireKey, iSecondEmpireKey);
    if (iErrCode != OK) {
        goto Cleanup;
    }

    iErrCode = DeleteAssociation (pEmpires, iSecondEmpireKey, iEmpireKey);
    if (iErrCode != OK) {
        Assert (iErrCode != ERROR_ASSOCIATION_NOT_FOUND);
        goto Cleanup;
    }

Cleanup:

    SafeRelease (pEmpires);

    return iErrCode;
}

int GameEngine::RemoveDeadEmpireAssociations (ICachedTable* pEmpires, unsigned int iEmpireKey) {

    int iErrCode;

    Variant vTemp;
    iErrCode = pEmpires->ReadData (iEmpireKey, SystemEmpireData::Associations, &vTemp);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (String::IsBlank(vTemp.GetCharPtr())) {
        return OK;
    }

    char* pszAssoc = (char*)StackAlloc (strlen (vTemp.GetCharPtr()) + 1);
    strcpy (pszAssoc, vTemp.GetCharPtr());

    unsigned int* piAssoc, i, iAssoc;
    iErrCode = GetAssociations (pszAssoc, &piAssoc, &iAssoc);
    if (iErrCode != OK) {
        return iErrCode;
    }

    if (iAssoc > 0) {

        for (i = 0; i < iAssoc; i ++) {

            iErrCode = DeleteAssociation (pEmpires, piAssoc[i], iEmpireKey);
            if (iErrCode != OK) {
                Assert (iErrCode != ERROR_ASSOCIATION_NOT_FOUND);
                OS::HeapFree (piAssoc);
                return iErrCode;
            }
        }

        OS::HeapFree (piAssoc);
    }

    return OK;
}
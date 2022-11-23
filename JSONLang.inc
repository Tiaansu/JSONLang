// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_JSONLang
    #undef _inc_JSONLang
#endif
// custom include-guard to ensure we don't duplicate
#if defined _JSON_lang_included
    #endinput
#endif
#define _JSON_lang_included

/**
 * 
 * This project is a JSON version of ScavengeSurvives' Language library. (https://github.com/ScavengeSurvive/language)
 */

#include <string>
#include <strlib>
#include <logger>
#include <map>
#include <fsutil>
#include <YSI_Coding\y_hooks>

// The directory language data is stored
#if !defined DIRECTORY_JSON_LANGUAGES
    #define DIRECTORY_JSON_LANGUAGES "I18n"
#endif

// The total amount of languages
#if !defined MAX_JSON_LANGUAGE
    #define MAX_JSON_LANGUAGE   (12)
#endif

// The total amount of entries per language
#if !defined MAX_JSON_LANGUAGE_ENTRIES
    #define MAX_JSON_LANGUAGE_ENTRIES   (1024)
#endif

// The maximum length of a language key
#if !defined MAX_JSON_LANGUAGE_KEY_LEN
    #define MAX_JSON_LANGUAGE_KEY_LEN   (32)
#endif

// The maximum length of a language text entry
#if !defined MAX_JSON_LANGUAGE_ENTRY_LENGTH
    #define MAX_JSON_LANGUAGE_ENTRY_LENGTH  (1024)
#endif

// Name limit for a language
#if !defined MAX_JSON_LANGUAGE_NAME
    #define MAX_JSON_LANGUAGE_NAME  (32)
#endif

// Maximum amount of string aliases
#if !defined MAX_JSON_LANGUAGE_ALIASES
    #define MAX_JSON_LANGUAGE_ALIASES   (48)
#endif

// Lengths for alias key/value
#define MAX_JSON_LANGUAGE_ALIAS_KEY_LEN   (32)
#define MAX_JSON_LANGUAGE_ALIAS_VAL_LEN   (32)

#define INVALID_JSON_LANGUAGE_ID (-1)

// @Lis shorthand for getting  a language string for a given player
// parameters are %0: playerid, %1: key.
#define @L(%0,%1) GetJsonLanguageString(GetPlayerJsonLanguage(%0), %1)

enum e_JSON_LANGUAGE_ENTRY_DATA
{
    JSON_LANG_KEY[MAX_JSON_LANGUAGE_KEY_LEN],
    JSON_LANG_VALUE[MAX_JSON_LANGUAGE_ENTRY_LENGTH]
};

enum e_JSON_LANGUAGE_TAG_ALIAS_DATA
{
    JSON_LANG_ALIAS_KEY[MAX_JSON_LANGUAGE_ALIAS_KEY_LEN],
    JSON_LANG_ALIAS_VAL[MAX_JSON_LANGUAGE_ALIAS_VAL_LEN]
};

static
    JSON_LANG_NAME[MAX_JSON_LANGUAGE][MAX_JSON_LANGUAGE_NAME],
    Map:JSON_LANG_MAP[MAX_JSON_LANGUAGE],
    JSON_LANG_TOTAL,

    JSON_LANG_REPLACEMENTS[MAX_JSON_LANGUAGE_ALIASES][e_JSON_LANGUAGE_TAG_ALIAS_DATA],
    JSON_LANG_TOTAL_REPLACEMENTS,

    JSON_LANG_PLAYER_LANGUAGE[MAX_PLAYERS]
;

// ---
// Hooked Callback(s)
// ---
hook OnPlayerConnect(playerid)
{
    JSON_LANG_PLAYER_LANGUAGE[playerid] = 0;
    return 1;
}

// ---
// Functions
// ---
stock DefineJsonLanguageReplacement(key[], val[])
{
    strcat(JSON_LANG_REPLACEMENTS[JSON_LANG_TOTAL_REPLACEMENTS][JSON_LANG_ALIAS_KEY], key, MAX_JSON_LANGUAGE_ALIAS_KEY_LEN);
    strcat(JSON_LANG_REPLACEMENTS[JSON_LANG_TOTAL_REPLACEMENTS][JSON_LANG_ALIAS_VAL], val, MAX_JSON_LANGUAGE_ALIAS_VAL_LEN);

    JSON_LANG_TOTAL_REPLACEMENTS++;
}

stock InitJsonLanguages()
{
    new
        Directory:dir = OpenDir(DIRECTORY_JSON_LANGUAGES),
        entry[256],
        ENTRY_TYPE:type,
        name[64]
    ;

    while (DirNext(dir, type, entry))
    {
        if (type == E_REGULAR)
        {
            PathBase(entry, name);

            Logger_Dbg("json_language", "loading json language file",
                Logger_S("entry", entry),
                Logger_S("name", name)
            );

            new id = InitJsonLanguageFromFile(name);
            if (id == -1)
            {
                Logger_Err("failed to load json languagage",
                    Logger_S("name", name)
                );
            }
        }
    }
    
    CloseDir(dir);
}

stock InitJsonLanguage(const name[])
{
    for (new json_lang_id; json_lang_id < JSON_LANG_TOTAL; ++ json_lang_id)
    {
        if (!strcmp(JSON_LANG_NAME[json_lang_id], name))
        {
            return json_lang_id;
        }
    }

    if (JSON_LANG_TOTAL == MAX_JSON_LANGUAGE)
    {
        Logger_Err("JSON_LANG_TOTAL reached MAX_JSON_LANGUAGGE",
            Logger_I("max", MAX_JSON_LANGUAGE)
        );
        return INVALID_JSON_LANGUAGE_ID;
    }

    new array[1][MAX_JSON_LANGUAGE_NAME];
    strexplode(array, name, ".", .ignorecase = true);

    strcat(JSON_LANG_NAME[JSON_LANG_TOTAL], array[0], MAX_JSON_LANGUAGE_NAME);

    Logger_Dbg("json_language", "initialized json language",
        Logger_S("name", array[0]),
        Logger_I("id", JSON_LANG_TOTAL)
    );

    return JSON_LANG_TOTAL++;
}

stock InitJsonLanguageFromFile(const json_lang_name[])
{
    new ret = initJsonLanguageFromFile(json_lang_name);

    // if we had a failure that wasn't caused by InitJsonLanguage
    // roll back JSON_LANG_TOTAL so a slot isn't used up.
    if (ret < -1)
    {
        JSON_LANG_TOTAL--;
    }
    return ret;
}

stock AddJsonLanguageEntry(json_lang_id, const key[], const value[])
{
    if (!(0 <= json_lang_id < JSON_LANG_TOTAL))
    {
        Logger_Err("attempt to add entry to invalid json language",
            Logger_I("json language id", json_lang_id),
            Logger_S("key", key),
            Logger_S("value", value)
        );
        return 1;
    }

    if (isnull(key))
    {
        Logger_Err("null key specified");
        return 2;
    }

    if (isnull(value))
    {
        Logger_Err("null value specified",
            Logger_S("key", key)
        );
        return 3;
    }

    if (MAP_count(JSON_LANG_MAP[json_lang_id]) >= MAX_JSON_LANGUAGE)
    {
        Logger_Err("MAX_JSON_LANGUAGE_ENTRIES limit reached",
            Logger_I("json language id", json_lang_id),
            Logger_S("key", key),
            Logger_S("value", value)
        );
        return 4;
    }

    new replaced[MAX_JSON_LANGUAGE_ENTRY_LENGTH];
    _json_language_doReplace(value, replaced);
    MAP_insert_str_str(JSON_LANG_MAP[json_lang_id], key, replaced);

    Logger_Dbg("json_language", "Added json language key",
        Logger_I("json language id", json_lang_id),
        Logger_S("key", key),
        Logger_S("value", value)
    );

    return 0;
}

stock GetJsonLanguageString(json_lang_id, const key[], bool:encode = false)
{
    new 
        result[MAX_JSON_LANGUAGE_ENTRY_LENGTH],
        ret
    ;

    if (!(0 <= json_lang_id < JSON_LANG_TOTAL))
    {
        Logger_Err("invalid json language",
            Logger_I("json language id", json_lang_id)
        );
        return result;
    }

    ret = _json_language_stringFromKey(json_lang_id, key, result, sizeof(result), encode);

    switch (ret)
    {
        case 1:
        {
            Logger_Err("invalid key",
                Logger_S("key", key)
            );
        }

        case 2:
        {
            Logger_Err("key not found",
                Logger_S("key", key),
                Logger_I("json language id", json_lang_id)
            );

            // return english if key not found
            if (json_lang_id != 0)
            {
                strcat(result, GetJsonLanguageString(0, key, encode), MAX_JSON_LANGUAGE_ENTRY_LENGTH);
            }
        }
    }
    return result;
}

stock GetJsonLanguageList(list[][])
{
    for (new i; i < JSON_LANG_TOTAL; i ++)
    {
        list[i][0] = EOS;
        strcat(list[i], JSON_LANG_NAME[i], MAX_JSON_LANGUAGE_NAME);
    }

    return JSON_LANG_TOTAL;
}

stock GetJsonLanguageName(json_lang_id, name[])
{
    if (!(0 <= json_lang_id < JSON_LANG_TOTAL))
    {
        return 1;
    }

    name[0] = EOS;
    strcat(name, JSON_LANG_NAME[json_lang_id], MAX_JSON_LANGUAGE_NAME);

    return 0;
}

stock GetJsonLanguageID(const name[])
{
    for (new i; i < JSON_LANG_TOTAL; i ++)
    {
        if (!strcmp(name, JSON_LANG_NAME[i]))
        {
            return i;
        }
    }
    
    return -1;
}

stock GetPlayerJsonLanguage(playerid)
{
    if (!IsPlayerConnected(playerid))
    {
        return -1;
    }

    return JSON_LANG_PLAYER_LANGUAGE[playerid];
}

stock SetPlayerJsonLanguage(playerid, json_lang_id)
{
    if (!IsPlayerConnected(playerid))
    {
        return 1;
    }

    JSON_LANG_PLAYER_LANGUAGE[playerid] = json_lang_id;

    return 0;
}

stock GetTotalJsonLanguage()
{
    return JSON_LANG_TOTAL;
}

// ---
// Internals
// ---
static stock _json_language_stringFromKey(json_lang_id, const key[], result[], len = sizeof(result), bool:encode = false)
{
    if (!MAP_contains_str(JSON_LANG_MAP[json_lang_id], key))
    {
        return 1;
    }

    MAP_get_str_arr(JSON_LANG_MAP[json_lang_id], key, result, len);

    if (encode)
    {
        ConvertEncoding(result);
    }

    return 0;
}

static stock _json_language_doReplace(const input[], output[])
{
    new
        bool:in_tag = false,
        tag_start = -1,
        output_idx
    ;

    for (new i = 0; input[i] != EOS; ++ i)
    {
        if (in_tag)
        {
            if (input[i] == '}')
            {
                for (new j; j < JSON_LANG_TOTAL_REPLACEMENTS; ++ j)
                {
                    if (!strcmp(input[tag_start], JSON_LANG_REPLACEMENTS[j][JSON_LANG_ALIAS_KEY], false, i - tag_start))
                    {
                        for (new k; JSON_LANG_REPLACEMENTS[j][JSON_LANG_ALIAS_VAL][k] != 0 && output_idx < MAX_JSON_LANGUAGE_ENTRY_LENGTH; ++ k)
                        {
                            output[output_idx++] = JSON_LANG_REPLACEMENTS[j][JSON_LANG_ALIAS_VAL][k];
                        }
                        break;
                    }
                }

                in_tag = false;
                continue;
            }
        }
        else
        {
            if (input[i] == '{')
            {
                tag_start = i + 1;
                in_tag = true;
                continue;
            }
            else if (input[i] == '\\')
            {
                if (input[i + 1] == 'n')
                {
                    output[output_idx++] = '\n';
                    i += 1;
                }
                else if (input[i + 1] == 't')
                {
                    output[output_idx++] = '\t';
                    i += 1;
                }
            }
            else
            {
                output[output_idx++] = input[i];
            }
        }
    }
}

static _json_language_quickSort(array[][], left, right)
{
    new
        tempLeft = left,
        tempRight = right,
        pivot = array[(left + right) / 2][0]
    ;

    while (tempLeft <= tempRight)
    {
        while (array[tempLeft][0] < pivot)
        {
            tempLeft++;
        }

        while (array[tempRight][0] > pivot)
        {
            tempRight--;
        }

        if (tempLeft <= tempRight)
        {
            _json_language_swapStrings(array[tempLeft][JSON_LANG_KEY], array[tempRight][JSON_LANG_KEY]);
            _json_language_swapStrings(array[tempLeft][JSON_LANG_VALUE], array[tempRight][JSON_LANG_ALIAS_VAL]);

            tempLeft++;
            tempRight--;
        }

        if (left < tempRight)
        {
            _json_language_quickSort(array, left, tempRight);
        }

        if (tempLeft < right)
        {
            _json_language_quickSort(array, tempLeft, right);
        }
    }
}

static _json_language_swapStrings(str1[], str2[])
{
    new tmp;

    for (new i; str1[i] != '\0' || str2[i] != '\0'; i ++)
    {
        tmp = str1[i];
        str1[i] = str2[i];
        str2[i] = tmp;
    }
}

/*
	Credit for this function goes to Y_Less:
	http://forum.sa-mp.com/showpost.php?p=3015480&postcount=6
*/

static stock ConvertEncoding(string[])
{
	static const
		real[256] =
        {
			0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
			16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
			32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
			48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
			64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
			80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
			96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
			112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
			128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
			144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
			160,  94, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
			124, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 175,
			128, 129, 130, 195, 131, 197, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141,
			208, 173, 142, 143, 144, 213, 145, 215, 216, 146, 147, 148, 149, 221, 222, 150,
			151, 152, 153, 227, 154, 229, 155, 156, 157, 158, 159, 160, 161, 162, 163, 164,
			240, 174, 165, 166, 167, 245, 168, 247, 248, 169, 170, 171, 172, 253, 254, 255
		};

	for(new i = 0, len = strlen(string), ch; i != len; ++i)
    {
		// Check if this character is in our reduced range.
		if(0 <= (ch = string[i]) < 256)
        {
			string[i] = real[ch];
		}
	}
}

static stock initJsonLanguageFromFile(const json_lang_name[])
{
    new json_lang_id = InitJsonLanguage(json_lang_name);
    if (json_lang_id == INVALID_JSON_LANGUAGE_ID)
    {
        return -1;
    }

    new
        filename[256],
        Node:node,
        ret,
        index
    ;

    format(filename, sizeof filename, DIRECTORY_JSON_LANGUAGES"%s", json_lang_name);
    ret = JSON_ParseFile(filename, node);

    if (ret > 0)
    {
        Logger_Err("unable to open file",
            Logger_S("filename", filename)
        );
        return -2;
    }

    new
        key[MAX_JSON_LANGUAGE_KEY_LEN],
        val[MAX_JSON_LANGUAGE_ENTRY_LENGTH]
    ;

    new Node:output, array[1][MAX_JSON_LANGUAGE_NAME];
    strexplode(array, json_lang_name, ".", .ignorecase = true);
    JSON_GetArray(node, array[0], output);

    new length;
    JSON_ArrayLength(output, length);

    for (new i = 0; i < length; i ++)
    {
        new Node:list;
        JSON_ArrayObject(output, i, list);

        JSON_GetString(list, "key", key);
        JSON_GetString(list, "value", val);

        AddJsonLanguageEntry(json_lang_id, key, val);
    }

    return index + 1;
}
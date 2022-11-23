# JSONLang

[![sampctl](https://img.shields.io/badge/sampctl-JSONLang-2f2f2f.svg?style=for-the-badge)](https://github.com/Tiaansu/JSONLang)

## Dependencies
* [YSI](https://github.com/pawn-lang/YSI-Includes)
* [samp-logger](https://github.com/Southclaws/samp-logger)
* [strlib](https://github.com/oscar-broman/strlib)
* [pawn-map](https://github.com/BigEti/pawn-map)
* [pawn-fsutil](https://github.com/Southclaws/pawn-fsutil)
* [pawn-json](https://github.com/Southclaws/pawn-json)

## Installation

Simply install to your project:

```bash
sampctl package install Tiaansu/JSONLang
```

Include in your code and begin using the library:

```pawn
#include <JSONLang>
```

## Usage

This package works by loading `json` files from `I18n` folder in the root directory.

These file should have the same keys, see the [`I18n`](https://github.com/Tiaansu/JSONLang/blob/main/I18n) directory in the repo for an example.

### Loading JSON Languages

To load a language from a json file, use `InitJsonLanguageFromFile` with just the language filename (including extension), not the full file path. For example `InitJsonLanguageFromFile("en-US.lang.json");` loads the `I18n/en-US.lang.json` file.

You could also combine this with the [`fsutil`](https://github.com/Southclaws/pawn-fsutil) plugin and iterate through the [`I18n`](https://github.com/Tiaansu/JSONLang/blob/main/I18n) directory.

```pawn
new
    Directory:dir = OpenDir("I18n"),
    entry[256],
    ENTRY_TYPE:type,
    name[64]
;

while (DirNext(dir, type, entry))
{
    if (type == E_REGULAR)
    {
        PathBase(entry, name);

        InitJsonLanguageFromFile(name);
    }
}

CloseDir(dir);
```

or just by simply using [`InitJsonLanguages()`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#118) in `OnGameModeInit`.

### Using JSON Language Strings

Now you've loaded json languages, you can use strings from each json language with the [`GetJsonLanguageString(json_lang_id, const key[], bool:encode = false)`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#244) function. The first parameter is the language ID, which you can obtain via name with [`GetJsonLanguageID`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#311).

For Example:

```pawn
SendClientMessage(playerid, -1, GetJsonLanguageString(GetJsonLanguageID("en-US"), "greet"));
```

Would send the string from `en-US` keyed by `greet`.

### Per-Player Language

You can store a language ID for each player with [`SetPlayerJsonLanguage`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#334) and retrieve it with [`GetPlayerJsonLanguage`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#324). For example, your server could display a list of languages in a dialog with [`GetJsonLanguageList`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#287) and when the player selects a language, call [`SetPlayerJsonLanguage`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#334) with their selection and then in future you can use [`GetPlayerJsonLanguage`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#324) to obtain the language ID that player selected.

There is also a useful macro [`@L(playerid, key[])`](https://github.com/Tiaansu/JSONLang/blob/main/JSONLang.inc#68) which you can use to quickly get a string for a player using their assigned language ID.

```pawn
SendClientMessage(playerid, -1, @L(playerid, "greet"));
```

## Testing

To test, simply run the package:

```bash
sampctl package run
```
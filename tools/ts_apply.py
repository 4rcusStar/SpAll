#!/usr/bin/env python3
"""Apply Chinese translations to a Qt .ts file.

Reads a JSON object mapping a message key to its translated text and writes those
translations into the .ts, clearing the `type="unfinished"` marker so lrelease accepts
them.

Two key forms are accepted, in this order of preference:

1. `"<context>\\u001f<source text>"` — matched on context *and* source text. Use this
   form: it survives a re-run of lupdate, which renumbers and reorders messages.
2. `"<index>"` — the flat message index printed by `ts_extract.py`. Convenient for a
   quick pass, but only valid for the exact .ts revision it was extracted from.

    python tools/ts_apply.py translations/spall_zh_CN.ts translations/zh_CN.json

Sources are compared after normalising line endings, so a translation survives the
harmless whitespace differences lupdate sometimes introduces.
"""

import json
import sys
import xml.etree.ElementTree as ET

# Unit separator: cannot occur in a context name or in normal source text, so it is a
# safe delimiter for the composite key.
SEP = "\x1f"


def normalise(text: str) -> str:
    return (text or "").replace("\r\n", "\n").replace("\r", "\n")


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 2

    ts_path, json_path = sys.argv[1], sys.argv[2]

    with open(json_path, encoding="utf-8") as handle:
        translations = json.load(handle)

    # Index the incoming map by both key forms.
    by_context_source = {}
    by_index = {}
    for key, value in translations.items():
        if SEP in key:
            ctx, _, source = key.partition(SEP)
            by_context_source[(ctx, normalise(source))] = value
        elif key.isdigit():
            by_index[int(key)] = value

    tree = ET.parse(ts_path)
    root = tree.getroot()

    applied = 0
    index = 0
    untranslated = []
    for context in root.findall("context"):
        name = context.findtext("name", default="")
        for message in context.findall("message"):
            translation = message.find("translation")
            source = message.findtext("source", default="")
            if translation is None:
                continue

            value = by_context_source.get((name, normalise(source)))
            if value is None:
                value = by_index.get(index)
            index += 1

            if value is None:
                untranslated.append(f"{name}: {source[:60]!r}")
                continue

            translation.text = value
            # lrelease ignores any message that keeps the unfinished marker.
            translation.attrib.pop("type", None)
            applied += 1

    ET.indent(tree, space="    ")
    tree.write(ts_path, encoding="utf-8", xml_declaration=True)

    # ElementTree writes the XML declaration but not Qt's DOCTYPE, and lrelease wants
    # it present. Re-insert it directly after the declaration.
    with open(ts_path, encoding="utf-8") as handle:
        content = handle.read()
    if "<!DOCTYPE TS>" not in content:
        first, _, rest = content.partition("\n")
        content = first + "\n<!DOCTYPE TS>\n" + rest
        with open(ts_path, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(content)

    print(f"applied {applied} translations to {ts_path}")
    if untranslated:
        print(f"still untranslated: {len(untranslated)}")
        for entry in untranslated[:25]:
            print("  " + entry)
    return 0


if __name__ == "__main__":
    sys.exit(main())

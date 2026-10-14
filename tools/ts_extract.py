#!/usr/bin/env python3
"""Extract translatable strings from a Qt .ts file into a flat, numbered work list.

Why this exists: the Chinese catalogue has ~257 entries across 11 contexts, far too
many to hand-edit reliably, and the .ts format is XML with quoting rules that make
sed-style editing dangerous. This script and its counterpart `ts_apply.py` give the
translation round-trip a safe, repeatable form:

    lupdate ...                                   # extract from sources
    python tools/ts_extract.py translations/spall_zh_CN.ts worklist.txt
    ... translate the work list ...
    python tools/ts_apply.py translations/spall_zh_CN.ts translations.json

Run the build's lupdate target first so the .ts matches the current sources.
"""

import sys
import xml.etree.ElementTree as ET


def main() -> int:
    if len(sys.argv) != 3:
        print(__doc__)
        return 2

    ts_path, out_path = sys.argv[1], sys.argv[2]

    tree = ET.parse(ts_path)
    root = tree.getroot()

    lines = []
    index = 0
    for context in root.findall("context"):
        name = context.findtext("name", default="")
        for message in context.findall("message"):
            source = message.findtext("source", default="")
            translation = message.find("translation")
            done = (
                translation is not None
                and translation.get("type") != "unfinished"
                and (translation.text or "").strip() != ""
            )
            state = "DONE" if done else "TODO"
            # Newlines inside a source string are written as a visible marker so each
            # entry stays on one line and the numbering survives round-tripping.
            flat = source.replace("\n", "\\n")
            lines.append(f"[{index}] ctx={name} [{state}] :: {flat}")
            index += 1

    with open(out_path, "w", encoding="utf-8", newline="\n") as handle:
        handle.write("\n".join(lines) + "\n")

    todo = sum(1 for line in lines if "[TODO]" in line)
    print(f"extracted {index} messages ({todo} untranslated) -> {out_path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
